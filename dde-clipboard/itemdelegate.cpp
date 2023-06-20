// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "itemdelegate.h"
#include "clipboardmodel.h"
#include "itemdata.h"
#include "iteminfo.h"
#include "itemwidget.h"

#include <QPointer>
#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QApplication>

DWIDGET_USE_NAMESPACE

static int caculateTextHeight(int width, int height)
{
    static const int textCenterWidth = ItemWidth - ContentMargin * 2;
    int extraline = 0;
    int over = width % textCenterWidth;
    if (over != 0) {
        extraline = 1;
    }
    auto caculated = width / textCenterWidth + extraline;
    int toShow = caculated > 4 ? 4 : caculated;
    return toShow * height;
}

static int getItemHeight(int width, int height)
{
    return ItemTitleHeight + ItemStatusBarHeight + TextContentTopMargin + caculateTextHeight(width, height);
}

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

    QPointer<ItemData> data = index.data().value<QPointer<ItemData>>();
    if (data->type() == DataType::Text) {
        QString text = data->text().simplified();
        int height = option.fontMetrics.height();
        int width = option.fontMetrics.horizontalAdvance(text);
        return QSize(ItemWidth, ItemMargin + getItemHeight(width, height));
    }
    return QSize(ItemWidth, ItemHeight + ItemMargin);
}

void ItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    QRect rect = option.rect;
    QPointer<ItemData> data = index.data().value<QPointer<ItemData>>();
    if (data->type() == DataType::Text) {
        QString text = data->text().simplified();
        int height = option.fontMetrics.height();
        int width = option.fontMetrics.horizontalAdvance(text);
        editor->setGeometry(rect.x() + ItemMargin, rect.y(), ItemWidth, getItemHeight(width, height));
        return;
    }
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
