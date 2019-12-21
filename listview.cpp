#include "listview.h"
#include "refreshtimer.h"

#include <QEvent>
#include <QKeyEvent>
#include <QDebug>
#include <QTimer>

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

void ListView::mouseMoveEvent(QMouseEvent *event)
{
    const QModelIndex index = indexAt(event->pos());
    if (index.isValid()) {
        setCurrentIndex(index);
    }
    return QListView::mouseMoveEvent(event);
}

void ListView::showEvent(QShowEvent *event)
{
    RefreshTimer::instance()->forceRefresh();
    activateWindow();

    QTimer::singleShot(0, this, [ = ] {
        scrollToTop();
        setCurrentIndex(model()->index(0, 0));
    });

    return QListView::showEvent(event);
}
