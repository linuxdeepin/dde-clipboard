// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wlrdatacontrolclipboardinterface.h"
#include "wlrdatacontrolofferintegration.h"
#include "dwaylandmimedata.h"
#include "dde-clipboard-daemon/constants.h"
#include <private/qwaylandnativeinterface_p.h>
#include <private/qwaylandintegration_p.h>
#include <private/qinternalmimedata_p.h>
#include <QGuiApplication>
#include <QImageReader>
#include <QImageWriter>
#include <QBuffer>
#include <QPixmap>
#include <QElapsedTimer>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <string.h>

using namespace Qt::StringLiterals;

constexpr qsizetype MaxMimePayloadSize = 64 * 1024 * 1024;
constexpr int PipeReadChunkSize = 4096;
constexpr int PipeReadTimeoutMs = 10 * 1000;
constexpr int PipePollIntervalMs = 1000;

// File descriptor close guard
class FdGuard {
public:
    FdGuard(int fd) : m_fd(fd) {}
    ~FdGuard()
    {
        if (m_fd >= 0)
            close(m_fd);
    }
private:
    int m_fd;
};

struct PipeReadRequest {
    QString mimeType;
    int fd = -1;
};

struct PipeReadPayload {
    QString mimeType;
    QByteArray data;
    bool success = false;
};

struct PipeReadState {
    QString mimeType;
    int fd = -1;
    QByteArray data;
    bool success = false;
    bool finished = false;
};

// Extra MIME Type Preprocessing
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

static inline QStringList imageWriteMimeFormats()
{
    return imageMimeFormats(QImageWriter::supportedImageFormats());
}

static inline bool isImageMimeType(const QString &mimeType)
{
    return mimeType.startsWith(QLatin1String("image/"));
}

static QStringList optimizedReadMimeTypes(const QStringList &mimeTypes)
{
    QStringList imageTypes;
    QStringList otherTypes;

    for (const QString &mimeType : mimeTypes) {
        if (isImageMimeType(mimeType))
            imageTypes.append(mimeType);
        else
            otherTypes.append(mimeType);
    }

    if (imageTypes.size() <= 1)
        return mimeTypes;

    QString preferredImageType;
    if (imageTypes.contains(QLatin1String("image/png"))) {
        preferredImageType = QLatin1String("image/png");
    } else if (imageTypes.contains(QLatin1String("image/jpeg"))) {
        preferredImageType = QLatin1String("image/jpeg");
    } else {
        preferredImageType = imageTypes.constFirst();
    }

    otherTypes.prepend(preferredImageType);
    return otherTypes;
}

static void closePipeState(PipeReadState &state)
{
    if (state.fd >= 0) {
        close(state.fd);
        state.fd = -1;
    }
}

static void finishPipeState(PipeReadState &state, bool success)
{
    state.success = success;
    state.finished = true;
    closePipeState(state);
}

static void closePipeStates(QList<PipeReadState> &states)
{
    for (PipeReadState &state : states)
        closePipeState(state);
}

static QList<PipeReadPayload> readPipeData(QList<PipeReadRequest> requests,
                                           const std::shared_ptr<std::atomic_bool> &cancelFlag)
{
    QList<PipeReadState> states;
    states.reserve(requests.size());
    for (const PipeReadRequest &request : requests)
        states.append({request.mimeType, request.fd});

    QElapsedTimer timer;
    timer.start();
    qsizetype activeCount = states.size();

    while (activeCount > 0) {
        if (cancelFlag && cancelFlag->load()) {
            closePipeStates(states);
            return {};
        }

        const qint64 elapsed = timer.elapsed();
        if (elapsed >= PipeReadTimeoutMs) {
            for (PipeReadState &state : states) {
                if (!state.finished)
                    qWarning() << "Timeout reading Wayland clipboard MIME:" << state.mimeType;
            }
            closePipeStates(states);
            break;
        }

        QList<pollfd> pollFds;
        QList<qsizetype> stateIndexes;
        pollFds.reserve(activeCount);
        stateIndexes.reserve(activeCount);

        for (qsizetype i = 0; i < states.size(); ++i) {
            const PipeReadState &state = states.at(i);
            if (state.finished)
                continue;

            pollfd pfd;
            pfd.fd = state.fd;
            pfd.events = POLLIN | POLLHUP | POLLERR;
            pfd.revents = 0;
            pollFds.append(pfd);
            stateIndexes.append(i);
        }

        const int waitMs = qMin(PipePollIntervalMs, int(PipeReadTimeoutMs - elapsed));
        const int ready = poll(pollFds.data(), static_cast<nfds_t>(pollFds.size()), waitMs);
        if (ready < 0) {
            if (errno == EINTR)
                continue;

            qWarning() << "Failed to poll Wayland clipboard pipes:" << strerror(errno);
            closePipeStates(states);
            break;
        }

        if (ready == 0)
            continue;

        for (qsizetype i = 0; i < pollFds.size(); ++i) {
            if (pollFds.at(i).revents == 0)
                continue;

            PipeReadState &state = states[stateIndexes.at(i)];
            if (pollFds.at(i).revents & (POLLERR | POLLNVAL)) {
                qWarning() << "Wayland clipboard pipe error for MIME:" << state.mimeType;
                finishPipeState(state, false);
                --activeCount;
                continue;
            }

            char buffer[PipeReadChunkSize];
            const ssize_t readSize = read(state.fd, buffer, sizeof(buffer));
            if (readSize < 0) {
                if (errno == EINTR)
                    continue;

                qWarning() << "Failed to read Wayland clipboard pipe for MIME"
                           << state.mimeType << strerror(errno);
                finishPipeState(state, false);
                --activeCount;
                continue;
            }

            if (readSize == 0) {
                finishPipeState(state, true);
                --activeCount;
                continue;
            }

            if (state.data.size() + readSize > MaxMimePayloadSize) {
                qWarning() << "Wayland clipboard MIME payload is too large:"
                           << state.mimeType << state.data.size() + readSize;
                finishPipeState(state, false);
                --activeCount;
                continue;
            }

            state.data.append(buffer, readSize);
        }
    }

    QList<PipeReadPayload> payloads;
    payloads.reserve(states.size());
    for (const PipeReadState &state : states)
        payloads.append({state.mimeType, state.data, state.success});

    return payloads;
}

static bool canStorePayload(const PipeReadPayload &payload)
{
    if (!payload.success || payload.data.isEmpty())
        return false;

    if (!isImageMimeType(payload.mimeType))
        return true;

    return !QImage::fromData(payload.data).isNull();
}

static WaylandMimeReadResult readMimeDataFromPipes(quint64 readTaskId,
                                                   QList<PipeReadRequest> requests,
                                                   std::shared_ptr<std::atomic_bool> cancelFlag)
{
    WaylandMimeReadResult result;
    result.readTaskId = readTaskId;

    const QList<PipeReadPayload> payloads = readPipeData(std::move(requests), cancelFlag);
    if (cancelFlag && cancelFlag->load())
        return result;

    for (const PipeReadPayload &payload : payloads) {
        if (!canStorePayload(payload)) {
            qWarning() << "Skip invalid Wayland clipboard MIME payload:" << payload.mimeType;
            continue;
        }

        result.payloads.append({payload.mimeType, payload.data});
    }

    return result;
}

// Used when a historical data is replayed (reborn) to retrieve desired data type from q plain QMimeData
static QByteArray getByteArray(QMimeData *mimeData, const QString &mimeType)
{
    QByteArray content;
    if (mimeType == QLatin1String("text/plain")) {
        content = mimeData->text().toUtf8();
    } else if (mimeData->hasImage()
               && (mimeType == QLatin1String("application/x-qt-image")
                   || mimeType.startsWith(QLatin1String("image/")))) {
        const QVariant imageData = mimeData->imageData();
        QImage image = qvariant_cast<QImage>(imageData);
        if (image.isNull()) {
            const QPixmap pixmap = qvariant_cast<QPixmap>(imageData);
            if (!pixmap.isNull())
                image = pixmap.toImage();
        }
        if (!image.isNull()) {
            QBuffer buf;
            buf.open(QIODevice::ReadWrite);
            QByteArray fmt = "PNG";
            if (mimeType.startsWith(QLatin1String("image/"))) {
                QByteArray imgFmt = mimeType.mid(6).toLower().toLatin1();
                if (QImageWriter::supportedImageFormats().contains(imgFmt))
                    fmt = imgFmt;
            }
            QImageWriter wr(&buf, fmt);
            if (wr.write(image))
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

WlrDataControlClipboardInterface::WlrDataControlClipboardInterface(QObject *parent)
    : QObject{parent}
{   
    // Use one read thread so clipboard read tasks are handled in order.
    m_readThreadPool.setMaxThreadCount(1);
    // Keep writes off the read thread to avoid blocking self-produced offers.
    m_writeThreadPool.setMaxThreadCount(2);

    m_dcManager = std::make_unique<WlrDataControlManagerIntegration>();

    // Wait until manager is ready, obtain device & source by then
    connect(m_dcManager.get(), &QWaylandClientExtension::activeChanged,
            this, &WlrDataControlClipboardInterface::onActiveChanged);

    // Clipboard read / write completion signal
    connect(&m_readTaskWatcher, &QFutureWatcher<WaylandMimeReadResult>::finished,
            this, &WlrDataControlClipboardInterface::onReadTaskFinished);
}

const QMimeData *WlrDataControlClipboardInterface::mimeData() const
{
    return m_mimeData.get();
}

void WlrDataControlClipboardInterface::setMimeData(QMimeData *mimeData)
{
    // Write clipboard Stage 1: Send MIME Type Offers
    if (!mimeData || mimeData->formats().isEmpty()) {
        delete mimeData;
        return;
    }

    // Invalidate pending async reads before replacing clipboard data.
    qInfo() << "Replacing Wayland clipboard data from local source, formats:" << mimeData->formats();
    cancelPendingRead();
    mimeData->setData(PrivateMimeSavedForWayland, QByteArray());
    m_mimeData = std::unique_ptr<QMimeData>(mimeData);
    takeoverClipboardDataSource();
    Q_EMIT dataChanged();
}

void WlrDataControlClipboardInterface::refreshDataControlSourceDevice()
{
    // Create data control device to monitor clipboard content changes
    onDataControlDeviceFinished();
}

void WlrDataControlClipboardInterface::takeoverClipboardDataSource()
{
    // Create a new data control source (and possibly destroy old one)
    m_dcSource = std::make_unique<WlrDataControlSourceIntegration>(m_dcManager->create_data_source());
    if (!m_dcSource) {
        qWarning() << "Cannot create Data Control Source. Clipboard write is aborted.";
        return;
    }
    connect(m_dcSource.get(), &WlrDataControlSourceIntegration::send,
            this, &WlrDataControlClipboardInterface::onSourceSend);

    QStringList offeredMimeTypes;
    auto offerMimeType = [this, &offeredMimeTypes](const QString &mimeType) {
        if (offeredMimeTypes.contains(mimeType))
            return;

        m_dcSource->offer(mimeType);
        offeredMimeTypes.append(mimeType);
    };

    // Send MIME type offers
    for (const QString &format : m_mimeData->formats()) {
        // 如果是application/x-qt-image类型则需要提供image的全部类型, 比如image/png
        if (u"application/x-qt-image"_s == format) {
            for (const auto &i : imageWriteMimeFormats())
                offerMimeType(i);
        } else {
            offerMimeType(format);
        }
    }
    qInfo() << "Take over Wayland clipboard ownership, offered MIME types:" << offeredMimeTypes;

    // Tell the compositor that the clipboard content has changed.
    // Next time someone pastes stuff, we'll receive a signal and enter Stage 2.
    m_dcDevice->set_selection(m_dcSource.get()->object());
}

quint64 WlrDataControlClipboardInterface::beginReadTask()
{
    // A new task id makes older async read results stale.
    ++m_activeReadTaskId;
    requestReadTaskCancel();
    return m_activeReadTaskId;
}

void WlrDataControlClipboardInterface::cancelPendingRead()
{
    // Bump the task id even if the worker cannot stop immediately.
    ++m_activeReadTaskId;
    requestReadTaskCancel();
}

void WlrDataControlClipboardInterface::requestReadTaskCancel()
{
    if (m_readCancelFlag)
        m_readCancelFlag->store(true);

    if (m_readTaskFuture.isRunning()) {
        qWarning() << "An ongoing Wayland clipboard read was aborted, active task id:" << m_activeReadTaskId;
        m_readTaskFuture.cancel();
    }
}

bool WlrDataControlClipboardInterface::isCurrentReadTask(quint64 readTaskId) const
{
    return readTaskId == m_activeReadTaskId;
}

void WlrDataControlClipboardInterface::completeRead(std::unique_ptr<QMimeData> mimeData)
{
    if (!mimeData || mimeData->formats().isEmpty()) {
        qWarning() << "Wayland clipboard read finished without valid MIME data.";
        return;
    }

    m_mimeData = std::move(mimeData);

    Q_EMIT dataChanged();

    // Take over the Wayland clipboard data source so data remains pasteable after the source app exits.
    // The private MIME type prevents the manager from reading back the offer it just published.
    m_mimeData->setData(PrivateMimeSavedForWayland, QByteArray());
    takeoverClipboardDataSource();
}

void WlrDataControlClipboardInterface::onDataControlDeviceFinished()
{
    // Abort an ongoing read, if any
    cancelPendingRead();

    auto waylandIface = static_cast<QtWaylandClient::QWaylandNativeInterface *>(qGuiApp->platformNativeInterface());
    m_dcDevice = std::make_unique<WlrDataControlDeviceIntegration>(m_dcManager->get_data_device(waylandIface->seat()));

    connect(m_dcDevice.get(), &WlrDataControlDeviceIntegration::finished,
            this, &WlrDataControlClipboardInterface::onDataControlDeviceFinished);
    connect(m_dcDevice.get(), &WlrDataControlDeviceIntegration::newSelection,
            this, &WlrDataControlClipboardInterface::onNewSelection);
}

void WlrDataControlClipboardInterface::onNewSelection(WlrDataControlOfferIntegration *offer)
{
    // Read clipboard procedures.
    // Stage 1 (1.1, 1.2) are in DataControlDevice.
    // Stage 2: DataControlDevice sends us an offer.
    if (!offer) {
        qWarning() << "Offer not valid";
        return;
    }

    // Delete offer object automatically.
    auto offerGuard = std::unique_ptr<WlrDataControlOfferIntegration>(offer);

    // Filter MIME types.
    auto mimeTypes = QStringList(offer->availableMimeTypes());
    qInfo() << "Received Wayland clipboard offer, MIME types:" << mimeTypes;

    // Detect recursion caused by saving clipboard content (see onReadTaskFinished for explanation)
    if (mimeTypes.contains(PrivateMimeSavedForWayland)) {
        qInfo() << "Ignore self-produced Wayland clipboard offer.";
        return;
    }

    for (auto it = mimeTypes.begin(); it != mimeTypes.end(); ) {
        // 根据窗管的要求，不读取纯大写、和不含'/'的字段，因为源窗口可能没有写入这些字段的数据，导致获取数据的线程一直等待。
        if ((it->contains("/") || it->toUpper() != *it)
            || *it == "FROM_DEEPIN_CLIPBOARD_MANAGER"
            || *it == "TIMESTAMP") {
            ++it;
        } else {
            // Remove such entries
            it = mimeTypes.erase(it);
        }
    }
    if (mimeTypes.isEmpty()) {
        qWarning() << "No acceptable MIME types found, exiting.";
        return;
    }

    const quint64 readTaskId = beginReadTask();
    auto cancelFlag = std::make_shared<std::atomic_bool>(false);
    m_readCancelFlag = cancelFlag;
    auto display = QtWaylandClient::QWaylandIntegration::instance()->display();
    if (!display) {
        qWarning() << "Cannot read Wayland clipboard: display is not available.";
        m_readCancelFlag.reset();
        return;
    }

    QList<PipeReadRequest> requests;
    const QStringList readMimeTypes = optimizedReadMimeTypes(mimeTypes);
    qInfo() << "Start Wayland clipboard read task:" << readTaskId
            << "offered MIME count:" << mimeTypes.size()
            << "accepted MIME types:" << mimeTypes
            << "read MIME types:" << readMimeTypes;
    for (const QString &mimeType : readMimeTypes) {
        // Open communication pipe
        int pipefd[2];
        if (pipe(pipefd) != 0) {
            qCritical() << "Failed to create pipe, errno =" << errno;
            continue;
        }

        fcntl(pipefd[0], F_SETFD, FD_CLOEXEC);
        fcntl(pipefd[1], F_SETFD, FD_CLOEXEC);

        // Tell it what MIME type we want, and close our copy of pipe write end
        offer->receive(mimeType, pipefd[1]);
        close(pipefd[1]);
        requests.append({mimeType, pipefd[0]});
    }

    if (requests.isEmpty()) {
        qWarning() << "Wayland clipboard read task has no pipe requests, task id:" << readTaskId;
        m_readCancelFlag.reset();
        return;
    }

    display->flushRequests();

    m_readTaskFuture = QtConcurrent::run(&m_readThreadPool, readMimeDataFromPipes, readTaskId, requests, cancelFlag);
    m_readTaskWatcher.setFuture(m_readTaskFuture);
}

void WlrDataControlClipboardInterface::onReadTaskFinished()
{
    const QFuture<WaylandMimeReadResult> future = m_readTaskWatcher.future();
    if (future.resultCount() == 0) {
        qWarning() << "Wayland clipboard read task finished without a result.";
        return;
    }

    const WaylandMimeReadResult result = future.result();
    // Drop results from reads that were replaced or canceled later.
    if (!isCurrentReadTask(result.readTaskId)) {
        qWarning() << "Discard stale Wayland clipboard read result, task id:"
                   << result.readTaskId << "active task id:" << m_activeReadTaskId
                   << "payload count:" << result.payloads.size();
        return;
    }

    m_readCancelFlag.reset();
    QStringList payloadMimeTypes;
    payloadMimeTypes.reserve(result.payloads.size());
    for (const WaylandMimePayload &payload : result.payloads)
        payloadMimeTypes.append(payload.mimeType);
    qInfo() << "Finish Wayland clipboard read task:" << result.readTaskId
            << "payload count:" << result.payloads.size()
            << "payload MIME types:" << payloadMimeTypes;

    auto mimeData = std::make_unique<DWaylandMimeData>();
    for (const WaylandMimePayload &payload : result.payloads)
        mimeData->setData(payload.mimeType, payload.data);

    completeRead(std::move(mimeData));
}

void WlrDataControlClipboardInterface::onSourceSend(QString mimeType, int fd)
{
    // Write clipboard Stage 3: dispatch write task.
    // This should be put into a thread because daemon itself will also reply on reading
    // the clipboard (design burden)
    qInfo() << "Wayland clipboard owner received data request, MIME type:" << mimeType;
    auto _ = QtConcurrent::run(&m_writeThreadPool, [](QByteArray data, int fd){
        FdGuard fdGuard(fd);
        QFile fdFile;
        if (!fdFile.open(fd, QFile::WriteOnly)) {
            qWarning() << "Cannot open pipe; error:" << fdFile.errorString();
            return;
        }
        if (!fdFile.isWritable()) {
            qWarning() << "Pipe file is not writable.";
            return;
        }

        fdFile.write(data);
    }, getByteArray(m_mimeData.get(), mimeType), fd);
}

void WlrDataControlClipboardInterface::onSourceCancelled()
{
    // Destroy the data source.
    m_dcSource = nullptr;
}
