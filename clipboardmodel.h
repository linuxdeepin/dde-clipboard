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
#ifndef CLIPBOARDMODEL_H
#define CLIPBOARDMODEL_H

#include <QAbstractListModel>
#include <QClipboard>
#include <QListView>

#include "itemdata.h"

class ClipboardModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ClipboardModel(QListView *list, QObject *parent = nullptr);

    const QList<ItemData *> data();

public Q_SLOTS:
    void clear();
    void destroy(ItemData *data);
    void reborn(ItemData *data);

Q_SIGNALS:
    void dataChanged();
    void dataReborn();      //收起剪贴板

protected:
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

protected slots:
    void clipDataChanged();

private:
    QList<ItemData *> m_data;
    QClipboard *m_board;
    QListView *m_list;
};

#endif // CLIPBOARDMODEL_H
