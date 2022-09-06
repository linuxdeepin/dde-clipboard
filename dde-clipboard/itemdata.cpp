// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "itemdata.h"

#include <QDebug>
#include <QApplication>
#include <QWidget>
#include <QDataStream>

#include <QLabel>

static inline QString textUriListLiteral() { return QStringLiteral("text/uri-list"); }
static inline QString textPlainLiteral() { return QStringLiteral("text/plain"); }
static inline QString applicationXQtImageLiteral() { return QStringLiteral("application/x-qt-image"); }

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
        if (data.fileIcon.isNull()) {
            continue;
        }
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

/*!
 * \~chinese \name remove
 * \~chinese \brief 将当前剪切块数据移除,调用此函数会发出ItemData::destroy的信号
 */
void ItemData::remove()
{
    emit destroy(this);
}

/*!
 * \~chinese \name popTop
 * \~chinese \brief 将当前剪切块置顶,调用此函数会发出ItemData::reborn的信号
 */
void ItemData::popTop()
{
    emit reborn(this);
}

const QSize &ItemData::pixSize() const
{
    return m_pixSize;
}
