#include "listview.h"
#include "refreshtimer.h"
#include "constants.h"

#include <QEvent>
#include <QKeyEvent>
#include <QDebug>
#include <QTimer>
#include <QPropertyAnimation>
#include <QScroller>

ListView::ListView(QWidget *parent)
    : QListView(parent)
{
    setAutoFillBackground(false);
    viewport()->setAutoFillBackground(false);

    setSelectionMode(QListView::SingleSelection);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);
    setVerticalScrollMode(QListView::ScrollPerPixel);

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

void ListView::startAni(int index)
{
    grabMouse();
    for (int i = index + 1; i < this->model()->rowCount(QModelIndex()); ++i) {
        CreateAnimation(i);
    }
    // FIXME:比动画时间稍微长一点，否则可能会造成mouseMoveEvent中崩溃
    QTimer::singleShot(AnimationTime + 10, this, [ = ] {
        releaseMouse();
        QModelIndex currentIndex = this->model()->index(index, 0);
        if (!currentIndex.isValid())
        {
            currentIndex = this->model()->index(index - 1, 0);
        }
        setCurrentIndex(currentIndex);
        scrollTo(currentIndex);
    });

    //TODO 显示的最下面一个widget应该是其下面的widget通过动画移动上来的，但其下面的widget此时有可能并没有被listview创建，这里需要一个假动画
}

void ListView::CreateAnimation(int idx)
{
    Q_ASSERT(idx > 0);
    const QModelIndex index = this->model()->index(idx, 0);

    Q_ASSERT(index.isValid());
    QWidget *widget = this->indexWidget(index);
    if (!widget) {
        qDebug() << "index widget not created,shoule be returned;";
        return;
    }

    QPropertyAnimation *ani = new QPropertyAnimation(widget, "pos", this);
    ani->setStartValue(widget->pos());
    ani->setEndValue(widget->pos() + QPoint(0, -210));
    ani->setDuration(AnimationTime);
    ani->start(QPropertyAnimation::DeleteWhenStopped);
}
