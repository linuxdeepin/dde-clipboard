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
#include "clipboardmodel.h"

#include <QApplication>
#include <QPointer>
#include <QDebug>

ClipboardModel::ClipboardModel(QListView *list, QObject *parent) : QAbstractListModel(parent)
    , m_board(QApplication::clipboard())
    , m_list(list)
{
    connect(m_board, &QClipboard::dataChanged, this, &ClipboardModel::clipDataChanged);
}

void ClipboardModel::clear()
{
    beginResetModel();
    m_data.clear();
    endResetModel();

    Q_EMIT dataAllCleared();
}

const QList<ItemData *> ClipboardModel::data()
{
    return m_data;
}

void ClipboardModel::reset()
{
    beginResetModel();
    endResetModel();
}

int ClipboardModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_data.size();
}

QVariant ClipboardModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(role)
    if (index.isValid() && index.row() < m_data.size()) {
        QPointer<ItemData> dt = m_data.at(index.row());
        return QVariant::fromValue(dt);
    }

    return QVariant();
}

Qt::ItemFlags ClipboardModel::flags(const QModelIndex &index) const
{
    if (index.isValid()) {
        if (m_list != nullptr) m_list->openPersistentEditor(index);
        return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
    }
    return QAbstractListModel::flags(index);
}

void ClipboardModel::removeData(ItemData *data)
{
    int row = m_data.indexOf(data);

    beginRemoveRows(QModelIndex(), row, row);
    auto item = m_data.takeAt(m_data.indexOf(data));
    endRemoveRows();

    item->deleteLater();

    Q_EMIT dataRemoved();
}

void ClipboardModel::extract(ItemData *data)
{
    int idx = m_data.indexOf(data);
    if (idx < 1)
        return;

    beginRemoveRows(QModelIndex(), idx, idx);
    m_data.removeOne(data);
    endRemoveRows();

    QMimeData *mimeData = new QMimeData;

    QMapIterator<QString, QByteArray> it(data->formatMap());
    while (it.hasNext()) {
        it.next();
        mimeData->setData(it.key(), it.value());
    }

    switch (data->type()) {
    case ItemData::Text:
        mimeData->setText(data->text());
        mimeData->setHtml(data->html().isEmpty() ? data->text() : data->html());
        break;
    case ItemData::Image:
        mimeData->setImageData(data->imageData());
        break;
    case ItemData::File:
        mimeData->setUrls(data->urls());
        break;
    }

    m_board->setMimeData(mimeData);

    data->deleteLater();
}

bool ClipboardModel::isDataValid(const QMimeData *data)
{
    //文本全是空格的情况需要过滤
    if (!data->hasText()
            && !data->hasUrls()
            && !data->hasHtml()
            && !data->hasColor()
            && !data->hasImage())
        return false;

    // -- 1有些程序不规范，复制空内容也写入了剪贴板
    if (data->hasHtml() && data->html().simplified().isEmpty()) {
        qDebug() << "html not valid";
        return false;
    }

    // -- 2
    if (data->hasText() && data->text().simplified().isEmpty()) {
        qDebug() << "text not valid";
        return false;
    }

    // -- 3
    if (data->hasImage()) {
        QPixmap pix = qvariant_cast<QPixmap>(data->imageData());
        if (pix.size() == QSize(0, 0)) {
            {
                qDebug() << "image not valid";
                return false;
            }
        }
    }

    // -- 4
    if (data->hasUrls() && data->urls().size() == 0) {
        qDebug() << "url not valid";
        return false;
    }

    return true;
}

void ClipboardModel::clipDataChanged()
{
    const QMimeData *mimeData = m_board->mimeData();
    ItemData *item = new ItemData(mimeData);
    if (item->type() == ItemData::Unknown) {
        item->deleteLater();
        return;
    }

    if (m_data.size()) {
        if (item->isEqual(m_data.first())) {
            item->deleteLater();
            return;
        }
    }

    beginInsertRows(QModelIndex(), 0, 0);

    connect(item, &ItemData::distory, this, &ClipboardModel::removeData);
    connect(item, &ItemData::reborn, this, &ClipboardModel::extract);
    m_data.push_front(item);

    endInsertRows();

    Q_EMIT dataAdded();
}
