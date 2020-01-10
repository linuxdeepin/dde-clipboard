/*
 * Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     fanpengcheng_cm <fanpengcheng_cm@deepin.com>
 *
 * Maintainer: fanpengcheng_cm <fanpengcheng_cm@deepin.com>
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
#include "clipboardloader.h"

#include <QApplication>
#include <QDBusMetaType>
#include <QDebug>
#include <QClipboard>
#include <QMimeData>

QString findImageFormat(const QList<QString> &formats)
{
    // Check formats in this order.
    static const QStringList imageFormats = QStringList()
                                            << QString("image/png")
                                            << QString("image/bmp")
                                            << QString("image/jpeg")
                                            << QString("image/gif");

    for (const auto &format : imageFormats) {
        if (formats.contains(format))
            return format;
    }

    return QString();
}


QByteArray Info2Buf(const ItemInfo &info)
{
    QByteArray buf;

    QByteArray iconBuf;
    if (info.m_formatMap.keys().contains("x-dfm-copied/file-icons")) {
        iconBuf = info.m_formatMap["x-dfm-copied/file-icons"];
    }

    QDataStream stream(&buf, QIODevice::WriteOnly);
    stream << info.m_formatMap
           << info.m_type
           << info.m_urls
           << info.m_hasImage;
    if (info.m_hasImage) {
        stream << info.m_variantImage;
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
    int type;
    QByteArray iconBuf;
    stream >> info.m_formatMap
           >> type
           >> info.m_urls
           >> info.m_hasImage;
    if (info.m_hasImage) {
        stream >> info.m_variantImage;
    }

    stream >> info.m_enable
           >> info.m_text
           >> info.m_createTime
           >> iconBuf;

    QDataStream stream2(&iconBuf, QIODevice::ReadOnly);
    for (int i = 0 ; i < info.m_urls.size(); ++i) {
        FileIconData data;
        stream2 >> data.cornerIconList >> data.fileIcon;
        info.m_iconDataList.push_back(data);
    }

    info.m_type = static_cast<DataType>(type);

    return info;
}

ClipboardLoader::ClipboardLoader()
    : m_board(qApp->clipboard())
{
    connect(m_board, &QClipboard::dataChanged, this, &ClipboardLoader::doWork);
}

void ClipboardLoader::dataReborned(const QByteArray &buf)
{
    ItemInfo info;
    info.m_variantImage = 0;

    info = Buf2Info(buf);

    // set
    QMimeData *mimeData = new QMimeData;

    QMapIterator<QString, QByteArray> it(info.m_formatMap);
    while (it.hasNext()) {
        it.next();
        mimeData->setData(it.key(), it.value());
    }

    // FIXME:这一部分理论上不用设置，但是实际上不设置发现图像无法reborn
    switch (info.m_type) {
    case DataType::Text:
        mimeData->setText(info.m_text);
        mimeData->setHtml(info.m_text);
        break;
    case DataType::Image:
        mimeData->setImageData(info.m_variantImage);
        break;
    case DataType::File:
        mimeData->setUrls(info.m_urls);
        break;
    default:
        break;
    }
    m_board->setMimeData(mimeData);
}

void ClipboardLoader::doWork()
{
    ItemInfo info;
    info.m_variantImage = 0;

    const QMimeData *mimeData = m_board->mimeData();

    // 转移系统剪贴板所有权时造成的两次内容变化不需要显示，以下为与系统约定好的标识
    if (mimeData->data("FROM_DEEPIN_CLIPBOARD_MANAGER") == "1") {
        qDebug() << "FROM_DEEPIN_CLIPBOARD_MANAGER";
        return;
    }

    // 过滤重复数据
    if (mimeData->data("TIMESTAMP") == m_lastTimeStamp
            && m_lastTimeStamp != QByteArray::fromHex("00000000")) {//FIXME:qq截图的时间戳不变，这里特殊处理
        qDebug() << mimeData->data("TIMESTAMP");
        return;
    }
    m_lastTimeStamp = mimeData->data("TIMESTAMP");

    if (mimeData->hasImage()) {
        info.m_variantImage = mimeData->imageData();
        if (info.m_variantImage.isNull())
            return;

        info.m_hasImage = true;
        info.m_type = Image;
    } else if (mimeData->hasUrls()) {
        info.m_urls = mimeData->urls();
        if (!info.m_urls.count())
            return;

        info.m_type = File;
    } else {
        if (mimeData->hasText()) {
            info.m_text = mimeData->text();
        } else if (mimeData->hasHtml()) {
            info.m_text = mimeData->html();
        } else {
            return;
        }

        if (info.m_text.isEmpty())
            return;

        info.m_type = Text;
    }
    info.m_createTime = QDateTime::currentDateTime();
    info.m_enable = true;

    bool bPixSaved = false;
    QString imageFormat = findImageFormat(mimeData->formats());
    if (!imageFormat.isEmpty()) {
        info.m_formatMap.insert(imageFormat, mimeData->data(imageFormat));
        bPixSaved = true;
    }
    for (int i = 0; i < mimeData->formats().size(); ++i) {
        if (bPixSaved
                && mimeData->formats()[i] != imageFormat
                && mimeData->formats()[i].startsWith("image/")) {
            continue;
        }

        info.m_formatMap.insert(mimeData->formats()[i], mimeData->data(mimeData->formats()[i]));
    }

    QByteArray buf;
    buf = Info2Buf(info);

    Q_EMIT dataComing(buf);
}

