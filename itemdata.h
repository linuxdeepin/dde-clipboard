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

/*!
 * ~chinese \class ItemData
 * ~chinese \brief 存放每个剪切块中的数据
 */
class ItemData : public QObject
{
    Q_OBJECT
public:
    explicit ItemData(const QMimeData *mimeData);

    /*!
     * \~chinese \brief 数据类型
     */
    enum DataType {
        Unknown,
        Text,
        Image,
        File
    };

    /*!
     * \~chinese \brief 提供剪切块属性的接口
     */
    QString title();                            // 类型名称
    QString subTitle();                         // 字符数，像素信息，文件名称（多个文件显示XXX等X个文件）
    const QList<QUrl> &urls();                  // 文件链接
    const QDateTime &time();                    // 复制时间
    const QString &html();                      // 富文本信息
    const QString &text();                      // 内容预览
    void setDataEnabled(bool enable) {m_enable = enable;}
    bool dataEnabled() {return m_enable;}
    QPixmap pixmap();                           // 缩略图
    const DataType &type() {return m_type;}
    const QVariant &imageData();
    const QMap<QString, QByteArray> &formatMap();
    const QList<FileIconData> &IconDataList();

    /*!
     * \~chinese \name remove
     * \~chinese \brief 将当前剪切块数据移除,调用此函数会发出ItemData::destroy的信号
     */
    void remove();
    /*!
     * \~chinese \name popTop
     * \~chinese \brief 将当前剪切块置顶,调用此函数会发出ItemData::reborn的信号
     */
    void popTop();

    /*!
     * \~chinese \name isEqual
     * \~chinese \brief 判断当前事件产生的数据是否和其他数据相同,用于数据的去重
     * \~chinese \param other 其他数据
     * \~chinese \return 相同返回true,不相同返回false
     */
    bool isEqual(const ItemData *other);
    /*!
     * \~chinese \name isValid
     * \~chinese \brief 转移系统剪贴板所有权时造成的两次内容变化是否要显示
     * \~chinese \return 相同需要显示true,不需要显示返回false
     */
    bool isValid();

Q_SIGNALS:
    /*!
     * \~chinese \name destroy
     * \~chinese \brief 点击剪切块上的关闭按钮时会调用ItemData::remove函数,产生该信号后关联的槽函数
     * \~chinese ClipboardModel::destroy会被执行删除数据
     * \~chinese \param data 需要删除的数据
     */
    void destroy(ItemData *data);
    /*!
     * \~chinese \name reborn
     * \~chinese \brief 双击剪切块时会发出该信号,会执行ItemData::popTop函数,发出ItemData::reborn
     * \~chinese 的信号,产生该信号后会将当前数据项置顶
     * \~chinese \param data 需要置顶的数据
     */
    void reborn(ItemData *data);

private:
    QMap<QString, QByteArray> m_formatMap;
    DataType m_type = Unknown;
    QList<QUrl> m_urls;
    QVariant m_variantImage;
    QString m_html;
    QString m_text;
    bool m_enable;
    QDateTime m_createTime;
    QList<FileIconData> m_iconDataList;
};

#endif // ITEMDATA_H
