// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifdef USE_DEEPIN_KF5_WAYLAND
#include "waylandcopyclient.h"
#include "serviceflow/servicemanager.h"
#include "serviceflow/waylandcopyservice.h"

#include <QImageReader>
#include <QMimeData>
#include <QMutexLocker>
#include <QThread>
#include <QDebug>

#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/event_queue.h>
#include <KWayland/Client/registry.h>
#include <KWayland/Client/seat.h>
#include <KWayland/Client/datacontroldevice.h>
#include <KWayland/Client/datacontrolsource.h>
#include <KWayland/Client/datacontroloffer.h>

#include <unistd.h>

static const QString ApplicationXQtImageLiteral QStringLiteral("application/x-qt-image");

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
    , m_mimeData(new DMimeData())
    , m_seat(nullptr)
    , m_manager(new CommandServiceManager())
    , m_registry(nullptr)
{
    init();
}

WaylandCopyClient::~WaylandCopyClient()
{
    m_connectionThread->quit();
    m_connectionThread->wait();
    m_connectionThreadObject->deleteLater();

    if (m_mimeData)
        m_mimeData->deleteLater();

    m_manager->unregisterService(m_writeServices[0]);
    delete m_manager;
    qDeleteAll(m_writeServices);
    qDeleteAll(m_runningRootService);
}

void WaylandCopyClient::init()
{
    // alows 10 service flow to run.
    m_manager->setMaxServiceCount(10);

    connect(m_connectionThreadObject, &ConnectionThread::connected, this, &WaylandCopyClient::onThreadConnected, Qt::UniqueConnection);
    m_connectionThreadObject->moveToThread(m_connectionThread);
    m_connectionThread->start();
    m_connectionThreadObject->initConnection();
}

void WaylandCopyClient::setupRegistry()
{
    if (m_registry)
        delete m_registry;

    m_registry = new Registry(this);
    connect(m_registry, &Registry::seatAnnounced, this, &WaylandCopyClient::onRegistrySeatAnnounced, Qt::UniqueConnection);
    connect(m_registry, &Registry::dataControlDeviceManagerAnnounced, this, &WaylandCopyClient::onDeviceManagerAnnounced, Qt::UniqueConnection);

    m_registry->setEventQueue(m_eventQueue);
    m_registry->create(m_connectionThreadObject);
    m_registry->setup();
}

void WaylandCopyClient::cleanServiceFlow(CommandService *service)
{
    auto root = m_manager->findRootService(service);
    Q_ASSERT(root);

    m_manager->unregisterService(root);
    m_runningRootService.removeAll(root);
    QList<CommandService *> services = {root};
    CommandService *header = root;
    while (auto next = header->nextService()) {
        services.append(next);
        header = next;
    }
    // clean service;
    qDeleteAll(services);
}

void WaylandCopyClient::wakePipeSyncCondition()
{
    m_pipeSyncCondition.wakeOne();
}

void WaylandCopyClient::onThreadConnected()
{
    if (m_eventQueue)
        delete m_eventQueue;

    m_eventQueue = new EventQueue(this);
    m_eventQueue->setup(m_connectionThreadObject);
    setupRegistry();
}

void WaylandCopyClient::onDataOffered(KWayland::Client::DataControlOfferV1 *offer)
{
    MimeDataFilterService *mimefilterService = new MimeDataFilterService();
    RequestReceiveService *requestReceiveService = new RequestReceiveService(this);
    ReadDataService *readDataService = new ReadDataService();
    SyncMimeDataService *syncMimeDataService = new SyncMimeDataService();

    mimefilterService->setNextService(requestReceiveService);
    requestReceiveService->setNextService(readDataService);
    readDataService->setNextService(syncMimeDataService);

    connect(syncMimeDataService, &SyncMimeDataService::finished, this, &WaylandCopyClient::onServiceFlowFinished);

    m_manager->registerService(mimefilterService);
    m_runningRootService.append(mimefilterService);
    m_manager->start(mimefilterService);

    PipeCommandMessage *msg = new PipeCommandMessage();
    msg->offer = offer;
    msg->mimeData = m_mimeData;
    msg->srcMimeTypes = offer->offeredMimeTypes();
    m_manager->appendFlowMessage(msg, mimefilterService);

    QMutexLocker locker(&m_pipeSyncMutex);
    m_pipeSyncCondition.wait(&m_pipeSyncMutex);
}

void WaylandCopyClient::onRegistrySeatAnnounced(quint32 name, quint32 version)
{
    if (!m_registry->isValid())
        return;
    m_seat = m_registry->createSeat(name, version, this);
}

void WaylandCopyClient::onDeviceManagerAnnounced(quint32 name, quint32 version)
{
    if (m_dataControlDeviceManager)
        m_dataControlDeviceManager->destroy();

    m_dataControlDeviceManager = m_registry->createDataControlDeviceManager(name, version, this);
    if (!m_dataControlDeviceManager->isValid())
        return;

    auto dataControlDevice = m_dataControlDeviceManager->getDataDevice(m_seat, this);
    if (dataControlDevice != m_dataControlDevice) {
        m_dataControlDevice = dataControlDevice;

        if (m_dataControlDevice) {
            connect(m_dataControlDevice, &DataControlDeviceV1::selectionCleared, this, [&] {
                    m_copyControlSource = nullptr;
                    sendOffer();
            });

            connect(m_dataControlDevice, &DataControlDeviceV1::dataOffered, this, &WaylandCopyClient::onDataOffered);
        }
    }
}

void WaylandCopyClient::onServiceFlowFinished(bool success)
{
    SyncMimeDataService *service = dynamic_cast<SyncMimeDataService *>(sender());
    if (!service)
        return;

    if (success) {
        // all data has updated.
        Q_EMIT dataChanged();
        sendOffer();
    }

    cleanServiceFlow(service);
}

const QMimeData* WaylandCopyClient::mimeData()
{
    return m_mimeData;
}

void WaylandCopyClient::setMimeData(QMimeData *mimeData)
{
    if (m_mimeData)
        m_mimeData->deleteLater();

    m_mimeData = mimeData;
    sendOffer();

    Q_EMIT dataChanged();
}

void WaylandCopyClient::sendOffer()
{
    if (m_copyControlSource) {
        m_copyControlSource->deleteLater();
        disconnect(m_copyControlSource);
    }

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

void WaylandCopyClient::onSendDataRequest(const QString &mimeType, qint32 fd)
{
    if (m_writeServices.isEmpty()) {
         CollectWrittenDataService *collectService = new CollectWrittenDataService();
         WriteDataToFDService *wfdService = new WriteDataToFDService();
         collectService->setNextService(wfdService);

         this->m_writeServices.insert(0, collectService);
         this->m_writeServices.insert(1, wfdService);

         this->m_manager->registerService(collectService);
         this->m_manager->start(collectService);
    }

    WriteDataMessage *writeMsg = new WriteDataMessage();
    writeMsg->fd = fd;
    writeMsg->mimeType = mimeType;
    writeMsg->mimeData = m_mimeData;
    m_manager->appendFlowMessage(writeMsg, this->m_writeServices[0]);
}

#endif
