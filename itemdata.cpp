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
        m_type = Image;
    } else if (mimeData->hasUrls()) {
        m_urls = mimeData->urls();
        m_type = File;
    } else {
        if (mimeData->hasHtml()) {
            m_text = mimeData->html();
        }
        if (mimeData->hasText()) {
            m_text = mimeData->text();
        }
        m_type = Text;
    }

    m_createTime = QDateTime::currentDateTime();

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
    }

    return "";
}

QString ItemData::subTitle()
{
    switch (m_type) {
    case Image:
        return "";
    case Text:
        return QString(tr("%1 characters")).arg(m_text.simplified().length());
    case File:
        return "";
    }

    return "";
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

void ItemData::setSelect(bool select)
{
    m_select = select;
}

void ItemData::remove()
{
    emit distory(this);
}

void ItemData::popTop()
{
    emit reborn(this);
}
