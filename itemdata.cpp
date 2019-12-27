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

ItemData::ItemData(const QMimeData *mimeData)
{
    if (mimeData->hasImage()) {
        m_variantImage = mimeData->imageData();
        if (m_variantImage.isNull())
            return;

        m_type = Image;
    } else if (mimeData->hasUrls()) {
        m_urls = mimeData->urls();
        if (!m_urls.count())
            return;

        m_type = File;
    } else {
        if (mimeData->hasText()) {
            m_text = mimeData->text();
        } else if (mimeData->hasHtml()) {
            m_text = mimeData->html();
        } else {
            return;
        }

        if (m_text.isEmpty())
            return;

        m_type = Text;
    }

    m_createTime = QDateTime::currentDateTime();

    if (mimeData->formats().contains("x-dfm-copied/file-icons")) {
        QByteArray buf = mimeData->data("x-dfm-copied/file-icons");
        QDataStream stream(&buf, QIODevice::ReadOnly);
        for (int i = 0; i < mimeData->urls().size(); ++i) {
            FileIconData data;
            stream >> data.cornerIconList >> data.fileIcon;
            //暂时不适用文件管理器提供的图标，其提供缩略图暂时有问题
            m_iconDataList.push_back(data);
        }
    }

    for (int i = 0; i < mimeData->formats().size(); ++i) {
        m_formatMap.insert(mimeData->formats()[i], mimeData->data(mimeData->formats()[i]));
#if 0
        qDebug() << mimeData->formats()[i] << mimeData->data(mimeData->formats()[i]);
#endif
    }
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

const QString &ItemData::html()
{
    return m_html;
}

const QString &ItemData::text()
{
    return m_text;
}

QPixmap ItemData::pixmap()
{
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

bool ItemData::isEqual(const ItemData *other)
{
    if (!m_formatMap["TIMESTAMP"].isEmpty()) {
        if (m_formatMap["TIMESTAMP"] == QByteArray::fromHex("00000000")) { //FIXME:qq截图的时间戳不变，这里特殊处理
            return false;
        } else if (m_formatMap["TIMESTAMP"] == other->m_formatMap["TIMESTAMP"]) {
#ifdef QT_DEBUG
            qDebug() << "equal the last";
#endif
            return true;
        }
    }

    return false;
}

bool ItemData::isValid()
{
    //转移系统剪贴板所有权时造成的两次内容变化不需要显示，以下为与系统约定好的标识
    if (m_formatMap["FROM_DEEPIN_CLIPBOARD_MANAGER"] == "1") {
#ifdef QT_DEBUG
        qDebug() << "not valid";
#endif
        return false;
    }

    return true;
}
