#include "listview.h"
#include "itemdata.h"

#include <QEvent>
#include <QKeyEvent>
#include <QPointer>
#include <QDebug>

ListView::ListView(QWidget *parent)
    : QListView(parent)
{
    viewport()->installEventFilter(this);
}

void ListView::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up: {
        QModelIndex currentIndex = this->currentIndex();
        QModelIndex targetIndex = currentIndex.sibling(currentIndex.row() - 1, 0);
        if (!currentIndex.isValid() || !targetIndex.isValid()) {
            targetIndex = model()->index(model()->rowCount() - 1, 0);
        }
        setCurrentIndex(targetIndex);
    }
    break;
    case Qt::Key_Down: {
        QModelIndex currentIndex = this->currentIndex();
        QModelIndex targetIndex = currentIndex.sibling(currentIndex.row() + 1, 0);
        if (!currentIndex.isValid() || !targetIndex.isValid()) {
            targetIndex = model()->index(0, 0);
        }
        setCurrentIndex(targetIndex);
    }
    break;
    default:
        QListView::keyPressEvent(event);
        break;
    }
}

void ListView::mouseMoveEvent(QMouseEvent *event)
{
    const QModelIndex &index = indexAt(event->pos());
    if (index.isValid()) {
        setCurrentIndex(index);
    }
}

void ListView::mousePressEvent(QMouseEvent *event)
{
    const QModelIndex index = indexAt(event->pos());
    Q_EMIT extract(index);

    return QListView::mousePressEvent(event);
}

void ListView::enterEvent(QEvent *event)
{
    activateWindow();

    return QListView::enterEvent(event);
}

void ListView::leaveEvent(QEvent *event)
{
    return QListView::leaveEvent(event);
}

bool ListView::eventFilter(QObject *obj, QEvent *event)
{
    Q_UNUSED(obj);
    //FIXME: viewport能接收到hovermove事件,但是listview却接收不到mousemove事件
    if (event->type() == QEvent::HoverMove) {
        if (QHoverEvent *hEvent = static_cast<QHoverEvent *>(event)) {
            QMouseEvent mEvent(QEvent::MouseMove, hEvent->pos(), Qt::NoButton, Qt::NoButton, hEvent->modifiers());
            mouseMoveEvent(&mEvent);
            return true;
        }
    }
    return false;
}

