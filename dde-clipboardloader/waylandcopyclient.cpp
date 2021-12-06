/*
 * Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     yinjie <yinjie@uniontech.com>
 *
 * Maintainer: yinjie <yinjie@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "waylandcopyclient.h"

#include <QEventLoop>
#include <QMimeData>
#include <QImageReader>
#include <QtConcurrent/QtConcurrent>

#include <KF5/KWayland/Client/connection_thread.h>
#include <KF5/KWayland/Client/event_queue.h>
#include <KF5/KWayland/Client/registry.h>
#include <KF5/KWayland/Client/seat.h>
#include <KF5/KWayland/Client/datacontroldevicemanager.h>
#include <KF5/KWayland/Client/datacontroldevice.h>
#include <KF5/KWayland/Client/datacontrolsource.h>
#include <KF5/KWayland/Client/datacontroloffer.h>

#include <unistd.h>

static QStringList imageMimeFormats(const QList<QByteArray> &imageFormats)
{
    QStringList formats;
    formats.reserve(imageFormats.size());
    for (const auto &format : imageFormats)
        formats.append(QLatin1String("image/") + QLatin1String(format.toLower()));

    // put png at the front because it is best
    int pngIndex = formats.indexOf(QLatin1String("image/png"));
    if (pngIndex != -1 && pngIndex != 0)
        formats.move(pngIndex, 0);

    return formats;
}

static inline QStringList imageReadMimeFormats()
{
    return imageMimeFormats(QImageReader::supportedImageFormats());
}

DMimeData::DMimeData()
{

}

DMimeData::~DMimeData()
{

}

QVariant DMimeData::retrieveData(const QString &mimeType, QVariant::Type preferredType) const
{
    QVariant data = QMimeData::retrieveData(mimeType,preferredType);
    if (mimeType == QLatin1String("application/x-qt-image")) {
        if (data.isNull() || (data.userType() == QMetaType::QByteArray && data.toByteArray().isEmpty())) {
            // try to find an image
            QStringList imageFormats = imageReadMimeFormats();
            for (int i = 0; i < imageFormats.size(); ++i) {
                data = QMimeData::retrieveData(imageFormats.at(i), preferredType);
                if (data.isNull() || (data.userType() == QMetaType::QByteArray && data.toByteArray().isEmpty()))
                    continue;
                break;
            }
        }
        int typeId = static_cast<int>(preferredType);
        // we wanted some image type, but all we got was a byte array. Convert it to an image.
        if (data.userType() == QMetaType::QByteArray
            && (typeId == QMetaType::QImage || typeId == QMetaType::QPixmap || typeId == QMetaType::QBitmap))
            data = QImage::fromData(data.toByteArray());
    } else if (mimeType == QLatin1String("application/x-color") && data.userType() == QMetaType::QByteArray) {
        QColor c;
        QByteArray ba = data.toByteArray();
        if (ba.size() == 8) {
            ushort * colBuf = (ushort *)ba.data();
            c.setRgbF(qreal(colBuf[0]) / qreal(0xFFFF),
                      qreal(colBuf[1]) / qreal(0xFFFF),
                      qreal(colBuf[2]) / qreal(0xFFFF),
                      qreal(colBuf[3]) / qreal(0xFFFF));
            data = c;
        } else {
            qWarning() << "Qt: Invalid color format";
        }
    } else if (data.userType() != int(preferredType) && data.userType() == QMetaType::QByteArray) {
        // try to use mime data's internal conversion stuf.
        DMimeData *that = const_cast<DMimeData *>(this);
        that->setData(mimeType, data.toByteArray());
        data = QMimeData::retrieveData(mimeType, preferredType);
        that->clear();
    }
    return data;
}

WaylandCopyClient::WaylandCopyClient(QObject *parent)
    : QObject(parent)
    , m_connectionThread(new QThread(this))
    , m_connectionThreadObject(new ConnectionThread())
    , m_eventQueue(nullptr)
    , m_dataControlDeviceManager(nullptr)
    , m_dataControlDevice(nullptr)
    , m_copyControlSource(nullptr)
    , m_mimeData(new DMimeData)
    , m_seat(nullptr)
{

}

WaylandCopyClient::~WaylandCopyClient()
{
    m_connectionThread->quit();
    m_connectionThread->wait();
    m_connectionThreadObject->deleteLater();
    if (m_mimeData)
        m_mimeData->deleteLater();
}

void WaylandCopyClient::init()
{
    connect(m_connectionThreadObject, &ConnectionThread::connected, this, [this] {
        m_eventQueue = new EventQueue(this);
        m_eventQueue->setup(m_connectionThreadObject);

        Registry *registry = new Registry(this);
        setupRegistry(registry);
    }, Qt::QueuedConnection );
    m_connectionThreadObject->moveToThread(m_connectionThread);
    m_connectionThread->start();
    m_connectionThreadObject->initConnection();
}

void WaylandCopyClient::setupRegistry(Registry *registry)
{
    connect(registry, &Registry::seatAnnounced, this, [this, registry] (quint32 name, quint32 version) {
        m_seat = registry->createSeat(name, version, this);
    });

    connect(registry, &Registry::dataControlDeviceManagerAnnounced, this, [this, registry] (quint32 name, quint32 version) {
        m_dataControlDeviceManager = registry->createDataControlDeviceManager(name, version, this);
        m_dataControlDevice = m_dataControlDeviceManager->getDataDevice(m_seat, this);

        connect(m_dataControlDevice, &DataControlDeviceV1::selectionCleared, this, [&] {
                m_copyControlSource = nullptr;
                setItmeInfo(m_itemInfo);
        });

        connect(m_dataControlDevice, &DataControlDeviceV1::dataOffered, this, &WaylandCopyClient::onDataOffered);
    });

    registry->setEventQueue(m_eventQueue);
    registry->create(m_connectionThreadObject);
    registry->setup();
}

void WaylandCopyClient::onDataOffered(KWayland::Client::DataControlOfferV1* offer)
{
    qDebug() << "data offered";
    if (!offer)
        return;

    if (m_mimeData)
        m_mimeData = new DMimeData();
    m_mimeData->clear();

    QList<QString> mimeTypeList = filterMimeType(offer->offeredMimeTypes());
    int mimeTypeCount = mimeTypeList.count();
    // 将所有的数据插入到mime data中
    for (const QString &mimeType : mimeTypeList) {
        int pipeFds[2];
        if (pipe(pipeFds) != 0) {
            qWarning() << "Create pipe failed.";
            return;
        }

        // 根据mime类取数据，写入pipe中
        qDebug() << "Write data to pipe";
        offer->receive(mimeType, pipeFds[1]);
        int retFd1 = close(pipeFds[1]);
        qDebug() << "Close pipe fd1 ret: " << retFd1;
        // 异步从pipe中读取数据写入mime data中
        QtConcurrent::run([pipeFds, this, mimeType, mimeTypeCount] {
            qDebug() << "read mimeType: " << mimeType;
            QFile readPipe;
            if (readPipe.open(pipeFds[0], QIODevice::ReadOnly)) {
                if (readPipe.isReadable()) {
                    const QByteArray &data = readPipe.readAll();
                    qDebug() << "Read pipe data finished: " << mimeType;
                    if (!data.isEmpty()) {
                        m_mimeData->setData(mimeType, data);
                    } else {
                        qWarning() << "Pipe data is empty, mime type: " << mimeType;
                    }
                } else {
                    qWarning() << "Pipe is not readable";
                }
            } else {
                qWarning() << "Open pipe failed!";
            }
            int retFd0 = close(pipeFds[0]);
            qDebug() << "close pipe fd0 ret: " << retFd0;
            static int mimeTypeIndex = 0;
            if (++mimeTypeIndex >= mimeTypeCount) {
                qDebug() << "emit dataChanged";
                emit this->dataChanged();
                mimeTypeIndex = 0;
            }
        });
    }
}

QMimeData* WaylandCopyClient::mimeData()
{
    return m_mimeData;
}

void WaylandCopyClient::setItmeInfo(const ItemInfo &info)
{
    m_itemInfo = info;
    m_copyControlSource = m_dataControlDeviceManager->createDataSource(this);
    if (!m_copyControlSource)
        return;

    connect(m_copyControlSource, &DataControlSourceV1::sendDataRequested, this, &WaylandCopyClient::onSendDataRequest);
    QMapIterator<QString, QByteArray> it(info.m_formatMap);
    while (it.hasNext()) {
        it.next();
        m_copyControlSource->offer(it.key());
    }

    m_dataControlDevice->setSelection(0, m_copyControlSource);
    m_connectionThreadObject->flush();
}

void WaylandCopyClient::onSendDataRequest(const QString &mimeType, qint32 fd) const
{
    auto it = m_itemInfo.m_formatMap.find(mimeType);
    if (it == m_itemInfo.m_formatMap.end())
        return;

    QFile f;
    if (f.open(fd, QFile::WriteOnly, QFile::AutoCloseHandle)) {
        f.write(it.value());
        f.close();
    }
}

QList<QString> WaylandCopyClient::filterMimeType(const QList<QString> &mimeTypeList)
{
    QList<QString> tmpList;
    for (const QString &mimeType : mimeTypeList) {
        // 根据窗管的要求，不读取纯大写、和不含'/'的字段，因为源窗口可能没有写入这些字段的数据，导致获取数据的线程一直等待。
        if ((mimeType.toUpper() != mimeType && mimeType.contains("/"))
                || mimeType == "FROM_DEEPIN_CLIPBOARD_MANAGER"
                || mimeType == "TIMESTAMP") {
            tmpList.append(mimeType);
        }
    }

    return tmpList;
}
