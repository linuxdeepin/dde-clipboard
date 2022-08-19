/*
 * Copyright (C) 2019 ~ 2022 Uniontech Technology Co., Ltd.
 *
 * Author:     fanpengcheng <fanpengcheng@uniontech.com>
 *
 * Maintainer: fanpengcheng <fanpengcheng@uniontech.com>
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
#ifndef ITEMINFO_H
#define ITEMINFO_H
#include <QDBusArgument>
#include <QDateTime>
#include <QUrl>

#include "constants.h"

enum DataType {
    Unknown,
    Text,
    Image,
    File
};

struct ItemInfo {
    QMap<QString, QByteArray> m_formatMap;
    DataType m_type = Unknown;
    QList<QUrl> m_urls;
    bool m_hasImage = false;
    QVariant m_variantImage;
    QSize m_pixSize;
    bool m_enable;
    QString m_text;
    QDateTime m_createTime;
    QList<FileIconData> m_iconDataList;
};

Q_DECLARE_METATYPE(ItemInfo)

#endif //ITEMINFO_H

