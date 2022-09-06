// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "itemdelegate.h"
#include "clipboardmodel.h"
#include "itemdata.h"
#include "itemwidget.h"

#include <QPointer>
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QApplication>

DWIDGET_USE_NAMESPACE

ItemDelegate::ItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(painter);
    Q_UNUSED(option);
    Q_UNUSED(index);
    return;
}

QWidget *ItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    QPointer<ItemData> data = index.data().value<QPointer<ItemData>>();
    ItemWidget *w = new ItemWidget(data, parent);
    w->installEventFilter(parent);
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

bool ItemDelegate::eventFilter(QObject *obj, QEvent *event)
{
    if (QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(event)) {
        switch (keyEvent->key()) {
        case Qt::Key_Tab:
        case Qt::Key_Backtab: if (keyEvent->type() == QKeyEvent::KeyPress) {
            //转变为特殊按键事件，表示切换内部‘焦点’，tab事件会被listview的viewport捕获
            QKeyEvent kEvent(QEvent::KeyPress, Qt::Key_0, Qt::NoModifier, "change focus");
            qApp->sendEvent(obj, &kEvent);
        }
        return true;
        default:
            break;
        }
    }

    return false;
}
