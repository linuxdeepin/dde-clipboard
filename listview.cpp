#include "listview.h"

#include <QEvent>
#include <QKeyEvent>
#include <QDebug>

ListView::ListView(QWidget *parent)
    : QListView(parent)
{
    setAutoFillBackground(false);
    viewport()->setAutoFillBackground(false);
    setSelectionMode(QListView::SingleSelection);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);
    setMouseTracking(true);
    viewport()->setMouseTracking(true);
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

void ListView::enterEvent(QEvent *event)
{
    activateWindow();

    return QListView::enterEvent(event);
}

void ListView::leaveEvent(QEvent *event)
{
    return QListView::leaveEvent(event);
}
