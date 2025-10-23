// Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "clipboardloader.h"

#include <QGuiApplication>
#include <QClipboard>
#include <QMimeData>
#include <QDir>
#include <QStandardPaths>
#include <QImageReader>
#include <QImageWriter>
#include <QBuffer>
#include <QSet>

const QString PixCacheDir = QStringLiteral("/clipboard-pix");  // 图片缓存目录名
const int MAX_BETYARRAY_SIZE = 10*1024*1024;    // 最大支持的文本大小
const int X11_PROTOCOL = 0;                     // x11协议
const int WAYLAND_PROTOCOL = 1;                 // wayland协议
const QString PngImageLiteral = QStringLiteral("image/png");  // PNG图片格式
const QByteArray CleanLastData = QByteArrayLiteral("CLEAN_LAST_DATA");  // 清除上次数据的标识

QByteArray Info2Buf(const ItemInfo &info)
{
    QByteArray buf;

    QByteArray iconBuf;
    if (info.m_formatMap.keys().contains("x-dfm-copied/file-icons")) {
        iconBuf = info.m_formatMap["x-dfm-copied/file-icons"];
    }

    QDataStream stream(&buf, QIODevice::WriteOnly);
    stream.setVersion(QDataStream::Qt_5_11);
    stream << info.m_formatMap
           << info.m_type
           << info.m_urls
           << info.m_hasImage;
    if (info.m_hasImage) {
        stream << info.m_variantImage;
        stream << info.m_pixSize;
    }
    stream  << info.m_enable
            << info.m_text
            << info.m_createTime
            << iconBuf;

    return buf;
}

ItemInfo Buf2Info(const QByteArray &buf)
{
    QByteArray tempBuf = buf;

    ItemInfo info;

    QDataStream stream(&tempBuf, QIODevice::ReadOnly);
    stream.setVersion(QDataStream::Qt_5_11);
    int type;
    QByteArray iconBuf;
    stream >> info.m_formatMap
           >> type
           >> info.m_urls
           >> info.m_hasImage;
    if (info.m_hasImage) {
        stream >> info.m_variantImage;
        stream >> info.m_pixSize;
    }

    stream >> info.m_enable
           >> info.m_text
           >> info.m_createTime
           >> iconBuf;

    QDataStream stream2(&iconBuf, QIODevice::ReadOnly);
    stream2.setVersion(QDataStream::Qt_5_11);
    for (int i = 0 ; i < info.m_urls.size(); ++i) {
        FileIconData data;
        stream2 >> data.cornerIconList >> data.fileIcon;
        info.m_iconDataList.push_back(data);
    }

    info.m_type = static_cast<DataType>(type);

    return info;
}

// 判断是否应该忽略保存的目标格式
static bool shouldIgnoreSaveTarget(const QString& format)
{
    // 这些系统格式不需要保存用于比对
    static QSet<QString> ignoreSaveTargetSet = {
        "SAVE_TARGETS", "TARGETS", "TIMESTAMP", "MULTIPLE",
        "DELETE", "INSERT_PROPERTY", "INSERT_SELECTION"
    };

    if (ignoreSaveTargetSet.contains(format))
        return true;

    // 对于图片格式，只保留特定的几种，其他的忽略
    if (format.startsWith("image/")) {
        if (format == "image/jpeg" ||
                format == "image/png" ||
                format == "image/bmp")
            return false;
        return true;
    }
    return false;
}

QString ClipboardLoader::m_pixPath;

ClipboardLoader::ClipboardLoader(QObject *parent)
    : QObject(parent)
    , m_board(nullptr)
    , m_wlrClipboard(nullptr)
{
    if (QStringLiteral("wayland") == qGuiApp->platformName()) {
        m_wlrClipboard = new WlrDataControlClipboardInterface(this);

        connect(m_wlrClipboard, &WlrDataControlClipboardInterface::dataChanged, this, [this] {
            this->doWork(WAYLAND_PROTOCOL);
        });
    } else {
        m_board = qApp->clipboard();
        connect(m_board, &QClipboard::dataChanged, this, [this] {
            this->doWork(X11_PROTOCOL);
        });
    }
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + PixCacheDir);
    if (dir.exists() && dir.removeRecursively()) {
        qDebug() << "ClipboardLoader startup, remove old cache, path:" << dir.path();
    }
}

void ClipboardLoader::dataReborned(const QByteArray &buf)
{
    // 当剪切板列表首条数据删除后，系统剪切板需要清除上次保存的格式，避免再次复制被过滤掉
    if (buf == CleanLastData) {
        m_clearLastData = true;
        m_lastFormatMap.clear();
        return;
    }

    ItemInfo info;
    info.m_variantImage = 0;
    info = Buf2Info(buf);

    QMimeData *mimeData = new QMimeData;
    QMapIterator<QString, QByteArray> it(info.m_formatMap);
    while (it.hasNext()) {
        it.next();
        mimeData->setData(it.key(), it.value());
    }

    switch (info.m_type) {
    case DataType::Image:
        setImageData(info, mimeData);
        break;
    default:
        break;
    }

    if (m_board) {
        m_board->setMimeData(mimeData);
    } else if (m_wlrClipboard) {
        m_wlrClipboard->setMimeData(mimeData);
    } else {
        Q_UNREACHABLE(); // Suppress memory leak warning
    }
}

void ClipboardLoader::extracted(const QMimeData *&mimeData, bool &dataChanged)
{
    for (auto f : mimeData->formats()) {
        // 跳过需要忽略的格式（系统格式和不需要的图片格式）
        if (shouldIgnoreSaveTarget(f))
            continue;

        // 需要先判断历史数据中是否存在此格式，否则使用m_lastFormatMap[f]时会返回默认空数据
        if (!m_lastFormatMap.contains(f)) {
            dataChanged = true;
            break;
        }

        // application/x-qt-image格式数据保存的为QPixmap对象，不能直接转换成QByteArray进行比对
        QByteArray data;
        if (f == ApplicationXQtImageLiteral) {
            const QPixmap &srcPix = qvariant_cast<QPixmap>(mimeData->imageData());
            if (!srcPix.isNull()) {
                QBuffer buffer(&data);
                buffer.open(QIODevice::WriteOnly);
                srcPix.save(&buffer, "PNG");
            }
        } else {
            data = mimeData->data(f);
        }

        // 先比对两次复制数据的大小后再比对具体内容
        if (m_lastFormatMap[f].size() != data.size()) {
            if (f == ApplicationXQtImageLiteral && data.size() == 0) {
                // qt 转换的数据可能没准备好, 防止刷数据。
                continue;
            }
            dataChanged = true;
            break;
        }

        if (m_lastFormatMap[f] != data) {
            dataChanged = true;
            break;
        }
    }
}

void ClipboardLoader::doWork(int protocolType)
{
    ItemInfo info;
    info.m_variantImage = 0;
    const bool clearLastData = m_clearLastData;
    m_clearLastData = false;

    // 快速复制时mimedata很可能是无效的(一般表现为获取的数据为空), 下面是qt的说明
    // The pointer returned might become invalidated when the contents
    // of the clipboard changes; either by calling one of the setter functions
    // or externally by the system clipboard changing.
    const QMimeData *mimeData = protocolType == WAYLAND_PROTOCOL ? m_wlrClipboard->mimeData() : m_board->mimeData();
    if (!mimeData || mimeData->formats().isEmpty())
        return;

    static qint64 lastOfferTime = 0;
    auto curFormats = mimeData->formats();
    auto lastFormats = m_lastFormatMap.keys();
    curFormats.sort();
    lastFormats.sort();
    bool listEqual = curFormats == lastFormats;
    auto offerDuration = QDateTime::currentMSecsSinceEpoch() - lastOfferTime;
    // 两次offer的时间间隔小于500ms，且list相同，不处理
    if (offerDuration < 500 &&
            listEqual &&
            !mimeData->formats().contains(ApplicationXQtImageLiteral)) {
        lastOfferTime = QDateTime::currentMSecsSinceEpoch();
        return;
    }
    lastOfferTime = QDateTime::currentMSecsSinceEpoch();

    // 适配厂商云桌面粘贴问题
    if (mimeData->formats().contains("uos/remote-copy")) {
        qDebug() << "FROM_SHENXINFU_CLIPBOARD_MANAGER";
        return;
    }

    // 转移系统剪贴板所有权时造成的两次内容变化不需要显示，以下为与系统约定好的标识
    if (mimeData->data("FROM_DEEPIN_CLIPBOARD_MANAGER") == "1") {
        qDebug() << "FROM_DEEPIN_CLIPBOARD_MANAGER";
        return;
    }

    // 过滤重复数据
    QByteArray currTimeStamp = mimeData->data("TIMESTAMP");
    if (currTimeStamp == m_lastTimeStamp
            && m_lastTimeStamp != QByteArray::fromHex("00000000")           // FIXME:TIM(wine)截图的时间戳不变，这里特殊处理
            && !currTimeStamp.isEmpty()
            && !clearLastData) {
        qDebug() << "TIMESTAMP:" << currTimeStamp << m_lastTimeStamp;
        return;
    }

    // 对比上次粘贴的数据，如果一样就不再粘贴
    bool dataChanged = false;
    if (listEqual || clearLastData) {
        extracted(mimeData, dataChanged);
    } else {
        // 对应场景，剪贴板双击复制置顶项后，再手动复制一样的内容，也需要判断是否是一样的数据
        // currentIsNormalData对应手动复制的内容，lastIsDoubleClickData对应上一次双击复制后的数据
        // 双击复制后返回的数据没有MULTIPLE和SAVE_TARGETS，以此区分
        bool currentIsNormalData = curFormats.contains("MULTIPLE") && curFormats.contains("SAVE_TARGETS");
        bool lastIsDoubleClickData = !lastFormats.isEmpty() && !lastFormats.contains("MULTIPLE") && !lastFormats.contains("SAVE_TARGETS");
        if (currentIsNormalData && lastIsDoubleClickData) {
            extracted(mimeData, dataChanged);
        } else {
            dataChanged = true;
        }
    }
    if (!dataChanged) {
        qDebug() << "Data is same, do not paste";
        return;
    }

    m_lastFormatMap.clear();
    for (const auto &format : mimeData->formats()) {
        // 对于需要忽略的格式，只记录格式名，不保存实际数据
        if (shouldIgnoreSaveTarget(format)) {
            m_lastFormatMap.insert(format, QByteArray());
            continue;
        }

        QByteArray data;
        // application/x-qt-image格式需要特殊处理：从QPixmap转换为PNG格式的QByteArray
        if (format == ApplicationXQtImageLiteral) {
            const QPixmap &srcPix = qvariant_cast<QPixmap>(mimeData->imageData());
            if (!srcPix.isNull()) {
                QBuffer buffer(&data);
                buffer.open(QIODevice::WriteOnly);
                srcPix.save(&buffer, "PNG");
            }
        } else {
            data = mimeData->data(format);
        }

        m_lastFormatMap.insert(format, data);
    }

    bool hasImage = false;
    QString imageFormat;
    auto supportedFormats = QImageWriter::supportedImageFormats();
    foreach (auto supportedFormat, supportedFormats) {
        imageFormat = "image/" + supportedFormat;
        if (mimeData->formats().contains(imageFormat)) {
            hasImage = true;
            break;
        }
    }

    if (mimeData->formats().contains(PngImageLiteral)) {
        hasImage = true;
        imageFormat = PngImageLiteral;
    }

    //图片类型的数据直接吧数据拿出来，不去调用mimeData->data()方法，会导致很卡
    if (mimeData->hasImage()) {
        const auto &srcImage = qvariant_cast<QImage>(mimeData->imageData()) ;
        if (srcImage.isNull()) {
            qDebug() << "mimeData->imageData()" << mimeData->imageData() << "QImage is null.";
            return;
        }

        info.m_pixSize = srcImage.size();
        if (!cachePixmap(srcImage, info)) {
            info.m_variantImage = srcImage;
        }

        info.m_formatMap.insert(ApplicationXQtImageLiteral, info.m_variantImage.toByteArray());
        info.m_formatMap.insert("TIMESTAMP", currTimeStamp);
        if (info.m_variantImage.isNull())
            return;

        // 正常数据时间戳不为空，这里增加判断限制 时间戳为空+图片内容不变 重复数据不展示
        // wayland下时间戳可能为空
        // 消除两次间隔小于500ms的重复图片数据
        if((currTimeStamp.isEmpty() || offerDuration < 500) && srcImage == m_lastImage && (QStringLiteral("wayland") != qGuiApp->platformName())) {
            qDebug() << "system repeat image";
            return;
        }
        m_lastImage = srcImage;

        info.m_hasImage = true;
        info.m_type = Image;
    } else if (hasImage) {
        // 部分情况下，应用(目前有截图录屏)发送的图片只有一种格式，例如image/png
        QPixmap srcPix;
        srcPix.loadFromData(mimeData->data(imageFormat));
        if (srcPix.isNull())
            return;

        info.m_pixSize = srcPix.size();
        if (!cachePixmap(srcPix.toImage(), info)) {
            info.m_variantImage = srcPix.toImage();
        }

        info.m_formatMap.insert(imageFormat, info.m_variantImage.toByteArray());
        info.m_formatMap.insert("TIMESTAMP", currTimeStamp);
        if (info.m_variantImage.isNull())
            return;

        // 正常数据时间戳不为空，这里增加判断限制 时间戳为空+图片内容不变 重复数据不展示
        // wayland下时间戳可能为空
        if(currTimeStamp.isEmpty() && srcPix.toImage() == m_lastImage && (QStringLiteral("wayland") != qGuiApp->platformName())) {
            qDebug() << "Current image from data is same as the last image, ignored";
            return;
        }
        m_lastImage = srcPix.toImage();

        info.m_hasImage = true;
        info.m_type = Image;
        info.m_createTime = QDateTime::currentDateTime();
        info.m_enable = true;

        m_lastTimeStamp = currTimeStamp;
    } else if (mimeData->hasUrls()) {
        info.m_urls = mimeData->urls();
        if (info.m_urls.isEmpty())
            return;

        //文件类型吧整个formats信息都拿出来，里面包含了文件的图标，以及文件的url数据等。
        for (const QString &format : mimeData->formats()) {
            const QByteArray &data = mimeData->data(format);
            if (!data.isEmpty())
                info.m_formatMap.insert(format, data);
        }
        if (info.m_formatMap.isEmpty())
            return;

        info.m_type = File;
    } else {
        if (mimeData->hasText()) {
            info.m_text = mimeData->text();
            // X11下，按住ctrl+c不放会出现hasText但是text/plain格式为空的情况，这里特殊处理一下
            if (info.m_text.isEmpty() && protocolType != WAYLAND_PROTOCOL)
                info.m_text = m_lastFormatMap.value("text/plain");
        } else if (mimeData->hasHtml()) {
            info.m_text = mimeData->html();
        } else {
            return;
        }

        const QByteArray &textByteArray = info.m_text.toUtf8();
        if (info.m_text.isEmpty() || textByteArray.size() > MAX_BETYARRAY_SIZE)
            return;

        // 保存所有数据，确保正常粘贴,缺少任意一种格式都可能导致粘贴失败
        for (auto f : mimeData->formats()) {
            if ("text/plain" == f && protocolType != WAYLAND_PROTOCOL) {
                info.m_formatMap.insert(f,  m_lastFormatMap.value("text/plain"));
                continue;
            }
            // 跳过需要忽略的格式
            if (shouldIgnoreSaveTarget(f))
                continue;

            info.m_formatMap.insert(f, mimeData->data(f));
        }

        info.m_type = Text;
    }

    info.m_createTime = QDateTime::currentDateTime();
    info.m_enable = true;

    m_lastTimeStamp = currTimeStamp;

    QByteArray buf;
    buf = Info2Buf(info);

    Q_EMIT dataComing(buf);
}

bool ClipboardLoader::cachePixmap(const QImage &srcPix, ItemInfo &info)
{
    if (initPixPath()) {
        QString pixFileName = m_pixPath + QString("/%1.png").arg(QDateTime::currentMSecsSinceEpoch());
        QImageWriter writer(pixFileName);
        if (!writer.canWrite() || !writer.write(srcPix)) {
            QFile cacheFile(pixFileName);
            if (!cacheFile.open(QIODevice::WriteOnly)) {
                qDebug() << "open file failed, file name:" << pixFileName;
                return false;
            }
            QDataStream stream(&cacheFile);
            stream.setVersion(QDataStream::Qt_5_11);
            stream << srcPix;
            cacheFile.close();
        }

        info.m_variantImage = srcPix.width() * PixmapHeight > srcPix.height() * PixmapWidth ?
                              srcPix.scaledToWidth(PixmapWidth, Qt::SmoothTransformation) :
                              srcPix.scaledToHeight(PixmapHeight, Qt::SmoothTransformation);

        // "text/uri-list":"file:///${XDG_CACHE_HOME}/deepin/dde-clipboard-daemon/clipboard-pix/xxx"
        info.m_formatMap.insert(TextUriListLiteral, QUrl::fromLocalFile(pixFileName).toEncoded());

        info.m_urls.push_back(QUrl::fromLocalFile(pixFileName));
        return true;
    }
    return false;
}

bool ClipboardLoader::initPixPath()
{
    if (!m_pixPath.isEmpty()) {
        return true;
    }

    QDir dir;
    m_pixPath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + PixCacheDir;
    if (dir.exists(m_pixPath)) {
        qDebug() << "dir exists:" << m_pixPath;
        return true;
    }

    if (dir.mkdir(m_pixPath)) {
        qDebug() << "mkdir:" << m_pixPath;
        return true;
    }

    qDebug() << "mkdir failed:" << m_pixPath;
    m_pixPath.clear();
    return false;
}

void ClipboardLoader::setImageData(const ItemInfo &info, QMimeData *&mimeData)
{
    //正常处理的图片会有一个缓存url
    if (info.m_urls.size() != 1) {
        qDebug() << "url size error, size:" << info.m_urls.size();
        mimeData->setImageData(info.m_variantImage);
        return;
    }

    const QString &fileName = info.m_urls.front().path();
    QImage cachedImage(fileName);
    if (cachedImage.isNull()) {
        qDebug() << "QImage failed to read cached image file" << fileName;
        mimeData->setImageData(info.m_variantImage);
        return;
    }

    mimeData->setImageData(cachedImage);
}
