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
#include "itemdelegate.h"
#include "clipboardmodel.h"
#include "itemdata.h"
#include "itemwidget.h"

#include <QPointer>

DWIDGET_USE_NAMESPACE

ItemDelegate::ItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

QWidget *ItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    QPointer<ItemData> data = index.data().value<QPointer<ItemData>>();
    ItemWidget *w = new ItemWidget(data, parent);
    return w;
}

QSize ItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    return QSize(ItemWidth, ItemHeight + ItemMargin);
}

void ItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    QRect rect = option.rect;
    editor->setGeometry(rect.x() + ItemMargin, rect.y(), ItemWidth, ItemHeight);
}
