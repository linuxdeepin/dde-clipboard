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

    Q_EMIT dataChanged();
}

const QList<ItemData *> ClipboardModel::data()
{
    return m_data;
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

void ClipboardModel::destroy(ItemData *data)
{
    int row = m_data.indexOf(data);

    beginRemoveRows(QModelIndex(), row, row);
    auto item = m_data.takeAt(m_data.indexOf(data));
    endRemoveRows();

    item->deleteLater();

    Q_EMIT dataChanged();
}

void ClipboardModel::reborn(ItemData *data)
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
    default:
        break;
    }

    m_board->setMimeData(mimeData);

    data->deleteLater();
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
        if (item->isEqual(m_data.first()) || !item->isValid()) {
            item->deleteLater();
            return;
        }
    }

    beginInsertRows(QModelIndex(), 0, 0);

    connect(item, &ItemData::destroy, this, &ClipboardModel::destroy);
    connect(item, &ItemData::reborn, this, &ClipboardModel::reborn);
    m_data.push_front(item);

    endInsertRows();

    Q_EMIT dataComing();
}
