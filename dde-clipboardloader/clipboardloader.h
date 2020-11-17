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
#ifndef WORKER_H
#define WORKER_H
#include <QObject>
#include <QClipboard>
#include <QDBusMetaType>
#include <QIcon>
#include <QDateTime>
#include <QList>
#include <QDebug>
#include <QDBusArgument>
#include <QDateTime>
#include <QUrl>

#include "constants.h"

typedef struct {
    QStringList cornerIconList;
    QIcon fileIcon;
} FileIconData;

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

class ClipboardLoader : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.dde.ClipboardLoader")
public:
    ClipboardLoader();

    bool cachePixmap(const QPixmap &srcPix, ItemInfo &info);
    void setImageData(const ItemInfo &info, QMimeData *&mimeData);

    static bool initPixPath();
public Q_SLOTS:
    void dataReborned(const QByteArray &buf);

private Q_SLOTS:
    void doWork();

private:
    QClipboard *m_board;
    QByteArray m_lastTimeStamp;
    QPixmap m_lastPix;

    static QString m_pixPath;

Q_SIGNALS:
    void dataComing(const QByteArray &buf);
};

#endif // WORKER_H
