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
#ifndef ITEMDATA_H
#define ITEMDATA_H

#include <QDateTime>
#include <QIcon>
#include <QMimeData>
#include <QObject>
#include <QPixmap>
#include <QUrl>

#include "constants.h"

class ItemData : public QObject
{
    Q_OBJECT
public:
    explicit ItemData(const QMimeData *mimeData);

    enum DataType {
        Text,
        Image,
        File
    };

    QString title();                            // 类型名称
    QString subTitle();                         // 字符数，像素信息，文件名称（多个文件显示XXX等X个文件）
    const QList<QUrl> &urls();                  // 文件链接
    const QDateTime &time();                    // 复制时间
    const QString &html();                      // 富文本信息
    const QString &text();                      // 内容预览
    QPixmap pixmap();                           // 缩略图
    const DataType &type() {return m_type;}
    const QVariant &imageData();
    const QMap<QString, QByteArray> &formatMap();

    bool select() { return m_select;}
    void setSelect(bool select);

    void remove();
    void popTop();

Q_SIGNALS:
    void distory(ItemData *data);
    void reborn(ItemData *data);

private:
    QMap<QString, QByteArray> m_formatMap;
    DataType m_type;
    QList<QUrl> m_urls;
    QVariant m_variantImage;
    QString m_html;
    QString m_text;
    QDateTime m_createTime;

    bool m_select = false;
};

#endif // ITEMDATA_H
