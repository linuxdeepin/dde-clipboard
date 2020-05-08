/*
 * Copyright (C) 2018 ~ 2025 Deepin Technology Co., Ltd.
 *
 * Author:     fanpengcheng <fanpengcheng_cm@deepin.com>
 *
 * Maintainer: fanpengcheng <fanpengcheng_cm@deepin.com>
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
#include "itemdata.h"

#include <QDebug>
#include <QApplication>
#include <QWidget>
#include <QDataStream>

#include <QLabel>

static inline QString textUriListLiteral() { return QStringLiteral("text/uri-list"); }
static inline QString textPlainLiteral() { return QStringLiteral("text/plain"); }
static inline QString applicationXQtImageLiteral() { return QStringLiteral("application/x-qt-image"); }

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

ItemData::ItemData(const QByteArray &buf)
{
    // get
    ItemInfo info;
    info = Buf2Info(buf);

    // convert
    QStringList formats = info.m_formatMap.keys();

    if (formats.contains(applicationXQtImageLiteral())) {
        m_variantImage = info.m_variantImage;
        if (m_variantImage.isNull())
            return;

        m_urls = info.m_urls;
        m_pixSize = info.m_pixSize;
        m_type = Image;
    } else if (formats.contains(textUriListLiteral())) {
        m_urls = info.m_urls;
        if (!m_urls.count())
            return;

        m_type = File;
    } else {
        if (formats.contains(textPlainLiteral())) {
            m_text = info.m_text;
        }  else {
            return;
        }

        if (m_text.isEmpty())
            return;

        m_type = Text;
    }

    m_createTime = QDateTime::currentDateTime();
    m_enable = true;
    m_iconDataList = info.m_iconDataList;
    m_formatMap = info.m_formatMap;
}

QString ItemData::title()
{
    switch (m_type) {
    case Image:
        return tr("Picture");
    case Text:
        return tr("Text");
    case File:
        return tr("File");
    default:
        return "";
    }
}

QString ItemData::subTitle()
{
    switch (m_type) {
    case Image:
        return "";
    case Text:
        return QString(tr("%1 characters")).arg(m_text.length());
    case File:
        return "";
    default:
        return "";
    }
}

const QList<QUrl> &ItemData::urls()
{
    return m_urls;
}

const QDateTime &ItemData::time()
{
    return m_createTime;
}

const QString &ItemData::text()
{
    return m_text;
}

void ItemData::setPixmap(const QPixmap &pixmap)
{
    m_thumnail = pixmap;
}

QPixmap ItemData::pixmap()
{
    if (!m_thumnail.isNull())
        return m_thumnail;

    QPixmap pix = qvariant_cast<QPixmap>(m_variantImage);
    return pix;
}

const QVariant &ItemData::imageData()
{
    return m_variantImage;
}

const QMap<QString, QByteArray> &ItemData::formatMap()
{
    return m_formatMap;
}

void ItemData::saveFileIcons(const QList<QPixmap> &list)
{
    m_fileIcons = list;
}

const QList<QPixmap> &ItemData::FileIcons()
{
    return m_fileIcons;
}

const QList<FileIconData> &ItemData::IconDataList()
{
    return m_iconDataList;
}

void ItemData::remove()
{
    emit destroy(this);
}

void ItemData::popTop()
{
    emit reborn(this);
}

const QSize &ItemData::pixSize() const
{
    return m_pixSize;
}
