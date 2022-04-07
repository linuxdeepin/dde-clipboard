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
#include "wayland_copy_client.h"

#include "constants.h"

#include <QEventLoop>
#include <QMimeData>
#include <QImageReader>
#include <QtConcurrent/QtConcurrent>
#include <QImageWriter>
#include <QMutexLocker>
#include <QMutex>

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

static QByteArray getByteArray(QMimeData *mimeData, const QString &mimeType)
{
    QByteArray content;
    if (mimeType == QLatin1String("text/plain")) {
        content = mimeData->text().toUtf8();
    } else if (mimeData->hasImage()
               && (mimeType == QLatin1String("application/x-qt-image")
                   || mimeType.startsWith(QLatin1String("image/")))) {
        QImage image = qvariant_cast<QImage>(mimeData->imageData());
        if (!image.isNull()) {
            QBuffer buf;
            buf.open(QIODevice::ReadWrite);
            QByteArray fmt = "BMP";
            if (mimeType.startsWith(QLatin1String("image/"))) {
                QByteArray imgFmt = mimeType.mid(6).toUpper().toLatin1();
                if (QImageWriter::supportedImageFormats().contains(imgFmt))
                    fmt = imgFmt;
            }
            QImageWriter wr(&buf, fmt);
            wr.write(image);
            content = buf.buffer();
        }
    } else if (mimeType == QLatin1String("application/x-color")) {
        content = qvariant_cast<QColor>(mimeData->colorData()).name().toLatin1();
    } else if (mimeType == QLatin1String("text/uri-list")) {
        QList<QUrl> urls = mimeData->urls();
        for (int i = 0; i < urls.count(); ++i) {
            content.append(urls.at(i).toEncoded());
            content.append('\n');
        }
    } else {
        content = mimeData->data(mimeType);
    }
    return content;
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
    } else {
        data = QMimeData::retrieveData(mimeType, preferredType);
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

WaylandCopyClient& WaylandCopyClient::ref()
{
    static WaylandCopyClient instance;

    return instance;
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
                sendOffer();
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
    static QMutex setMimeDataMutex;
    static int mimeTypeIndex = 0;
    mimeTypeIndex = 0;
    for (const QString &mimeType : mimeTypeList) {
        int pipeFds[2];
        if (pipe(pipeFds) != 0) {
            qWarning() << "Create pipe failed.";
            return;
        }

        // 根据mime类取数据，写入pipe中
        offer->receive(mimeType, pipeFds[1]);
        close(pipeFds[1]);
        // 异步从pipe中读取数据写入mime data中
        QtConcurrent::run([pipeFds, this, mimeType, mimeTypeCount] {
            QFile readPipe;
            if (readPipe.open(pipeFds[0], QIODevice::ReadOnly)) {
                if (readPipe.isReadable()) {
                    const QByteArray &data = readPipe.readAll();
                    if (!data.isEmpty()) {
                        // 需要加锁进行同步，否则可能会崩溃
                        QMutexLocker locker(&setMimeDataMutex);
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
            close(pipeFds[0]);
            if (++mimeTypeIndex >= mimeTypeCount) {
                qDebug() << "emit dataChanged";
                mimeTypeIndex = 0;
                emit this->dataChanged();
            }
        });
    }
}

const QMimeData* WaylandCopyClient::mimeData()
{
    return m_mimeData;
}

void WaylandCopyClient::setMimeData(QMimeData *mimeData)
{
    m_mimeData = mimeData;
    sendOffer();
}

void WaylandCopyClient::sendOffer()
{
    m_copyControlSource = m_dataControlDeviceManager->createDataSource(this);
    if (!m_copyControlSource)
        return;

    connect(m_copyControlSource, &DataControlSourceV1::sendDataRequested, this, &WaylandCopyClient::onSendDataRequest);
    for (const QString &format : m_mimeData->formats()) {
        // 如果是application/x-qt-image类型则需要提供image的全部类型, 比如image/png
        if (ApplicationXQtImageLiteral == format) {
            QStringList imageFormats = imageReadMimeFormats();
            for (int i = 0; i < imageFormats.size(); ++i) {
                m_copyControlSource->offer(imageFormats.at(i));
            }
            continue;
        }
        m_copyControlSource->offer(format);
    }

    m_dataControlDevice->setSelection(0, m_copyControlSource);
    m_connectionThreadObject->flush();
}

void WaylandCopyClient::onSendDataRequest(const QString &mimeType, qint32 fd) const
{
    QFile f;
    if (f.open(fd, QFile::WriteOnly, QFile::AutoCloseHandle)) {
        const QByteArray &ba = getByteArray(m_mimeData, mimeType);
        f.write(ba);
        f.close();
    }
}

QList<QString> WaylandCopyClient::filterMimeType(const QList<QString> &mimeTypeList)
{
    QList<QString> tmpList;
    for (const QString &mimeType : mimeTypeList) {
        // 根据窗管的要求，不读取纯大写、和不含'/'的字段，因为源窗口可能没有写入这些字段的数据，导致获取数据的线程一直等待。
        if ((mimeType.contains("/") && mimeType.toUpper() != mimeType)
                || mimeType == "FROM_DEEPIN_CLIPBOARD_MANAGER"
                || mimeType == "TIMESTAMP") {
            tmpList.append(mimeType);
        }
    }

    return tmpList;
}
