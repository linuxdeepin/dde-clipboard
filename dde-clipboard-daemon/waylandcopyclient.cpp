// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "waylandcopyclient.h"

#include <private/qwaylandnativeinterface_p.h>
#include <qguiapplication_platform.h>
#include <qnativeinterface.h>

#include <QBuffer>
#include <QDebug>
#include <QFile>
#include <QGuiApplication>
#include <QImageReader>
#include <QImageWriter>
#include <QTimer>

#include <unistd.h>

constexpr int BYTE_MAX = 1024 * 4;

static ::wl_display *DISPLAY = NULL;

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

static void display_flush()
{

    if (DISPLAY == NULL) {
        QtWaylandClient::QWaylandNativeInterface *app =
                static_cast<QtWaylandClient::QWaylandNativeInterface *>(
                        QGuiApplication::platformNativeInterface());
        DISPLAY = app->display();
    }
    wl_display_flush(DISPLAY);
}

ZWaylandDataControlManager::ZWaylandDataControlManager(QObject *parent)
    : QWaylandClientExtensionTemplate<ZWaylandDataControlManager>(2)
    , QtWayland::zwlr_data_control_manager_v1()
{
    QTimer::singleShot(0, this, &ZWaylandDataControlManager::initBase);
}

void ZWaylandDataControlManager::initBase()
{
    QtWaylandClient::QWaylandNativeInterface *app =
            static_cast<QtWaylandClient::QWaylandNativeInterface *>(
                    QGuiApplication::platformNativeInterface());

    auto seat = app->seat();
    auto data_device = get_data_device(seat);
    m_device = new ZDataControlDeviceV1(data_device, this);
    connect(m_device,
            &ZDataControlDeviceV1::clipboardChanged,
            this,
            &ZWaylandDataControlManager::clipboardChanged);
}

void ZWaylandDataControlManager::setMimeData(QMimeData *mimeData)
{
    if (m_device == NULL) {
        return;
    }
    if (mimeData->formats().isEmpty()) {
        return;
    }
    auto source = create_data_source();
    auto resouce = new ZDataControlResourceV1(source, mimeData, this);
    m_device->set_selection(source);
    for (const QString &format : mimeData->formats()) {
        // 如果是application/x-qt-image类型则需要提供image的全部类型, 比如image/png
        if (ApplicationXQtImageLiteral == format) {
            QStringList imageFormats = imageReadMimeFormats();
            for (int i = 0; i < imageFormats.size(); ++i) {
                resouce->offer(imageFormats.at(i));
            }
            continue;
        }
        resouce->offer(format);
    }

    display_flush();
}

ZDataControlResourceV1::ZDataControlResourceV1(::zwlr_data_control_source_v1 *resource,
                                               QMimeData *data,
                                               QObject *parent)

    : QObject(parent)
    , QtWayland::zwlr_data_control_source_v1(resource)
    , m_mimeData(data)
{
}

void ZDataControlResourceV1::zwlr_data_control_source_v1_send(const QString &mime_type, int32_t fd)
{
    QFile f;
    if (f.open(fd, QFile::WriteOnly, QFile::AutoCloseHandle)) {
        const QByteArray &ba = getByteArray(m_mimeData, mime_type);
        f.write(ba);
        f.close();
    }
    this->destroy();
    deleteLater();
    m_mimeData->deleteLater();
}

ZDataControlDeviceV1::ZDataControlDeviceV1(::zwlr_data_control_device_v1 *device, QObject *parent)
    : QObject(parent)
    , QtWayland::zwlr_data_control_device_v1(device)
    , m_data(new QMimeData)
{
}

ZDataControlOfferV1::ZDataControlOfferV1(::zwlr_data_control_offer_v1 *offer, QObject *parent)
    : QObject(parent)
    , QtWayland::zwlr_data_control_offer_v1(offer)
{
}

void ZDataControlOfferV1::zwlr_data_control_offer_v1_offer(const QString &mime_type)
{
    m_mimetypes.push_back(mime_type);
}

void ZDataControlDeviceV1::zwlr_data_control_device_v1_selection(
        struct ::zwlr_data_control_offer_v1 *id)
{
    if (id == NULL) {
        return;
    }
    if (m_dataoffer->m_mimetypes.isEmpty()) {
        return;
    }
    m_data->clear();
    for (auto &mime_type : m_dataoffer->m_mimetypes) {
        int pipeFds[2];
        if (pipe(pipeFds) != 0) {
            return;
        }
        auto [read, write] = pipeFds;
        m_dataoffer->receive(mime_type, write);

        close(write);

        // NOTE: need to flush finish the pipe
        display_flush();

        QByteArray data;
        if (readData(read, data)) {
            m_data->setData(mime_type, data);
        }
    }
    Q_EMIT clipboardChanged(m_data);
}

bool ZDataControlDeviceV1::readData(int fd, QByteArray &data)
{
    QFile readPipe;
    if (!readPipe.open(fd, QIODevice::ReadOnly)) {
        return false;
    }

    if (!readPipe.isReadable()) {
        qWarning() << "Pipe is not readable";
        readPipe.close();
        return false;
    }

    int retCount = BYTE_MAX;
    do {
        QByteArray bytes = readPipe.read(BYTE_MAX);
        retCount = bytes.length();
        if (!bytes.isEmpty())
            data.append(bytes);
    } while (retCount == BYTE_MAX);

    readPipe.close();

    return true;
}

void ZDataControlDeviceV1::zwlr_data_control_device_v1_primary_selection(
        [[maybe_unused]] struct ::zwlr_data_control_offer_v1 *id)
{
}

void ZDataControlDeviceV1::zwlr_data_control_device_v1_data_offer(
        struct ::zwlr_data_control_offer_v1 *id)
{
    if (!m_dataoffer.isNull()) {
        m_dataoffer->destroy();
    }

    m_dataoffer.reset(new ZDataControlOfferV1(id, this));
}
