#include "mainwindow.h"

// 边距
#define WINDOW_MARGIN 10

// 隐藏后的剩余空间
#define WINDOW_LEAVE 3

#define WINDOW_WIDTH 300

MainWindow::MainWindow(QWidget *parent)
    : DBlurEffectWidget(parent)
    , m_displayInter(new DBusDisplay(this))
    , m_model(new ClipboardModel)
{
    setWindowFlags(Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint);

    initConnect();

    geometryChanged();
}

MainWindow::~MainWindow()
{

}

void MainWindow::initConnect()
{
    connect(m_displayInter, &DBusDisplay::PrimaryRectChanged, this, &MainWindow::geometryChanged, Qt::QueuedConnection);

    connect(m_model, &ClipboardModel::dataAdded, this, &MainWindow::itemAdded);
    connect(m_model, &ClipboardModel::dataRemoved, this, &MainWindow::itemRemoved);
}

void MainWindow::enterEvent(QEvent *event)
{
    if (pos().x() < 0)
        move(WINDOW_MARGIN, WINDOW_MARGIN);

    DBlurEffectWidget::enterEvent(event);
}

void MainWindow::leaveEvent(QEvent *event)
{
    if (pos().x() > 0 && cursor().pos().x() > WINDOW_MARGIN)
        move(-width() + WINDOW_LEAVE, WINDOW_MARGIN);

    DBlurEffectWidget::leaveEvent(event);
}

void MainWindow::itemAdded(ItemData *item)
{

}

void MainWindow::itemRemoved(ItemData *item)
{

}

void MainWindow::geometryChanged()
{
    // 屏幕尺寸
    QRect rect = m_displayInter->primaryRawRect();
    rect.setWidth(WINDOW_WIDTH);
    rect.moveLeft(-rect.width() + WINDOW_MARGIN + WINDOW_LEAVE);
    m_geometry = rect.marginsRemoved(QMargins(WINDOW_MARGIN, WINDOW_MARGIN, WINDOW_MARGIN, WINDOW_MARGIN));
    setGeometry(m_geometry);
}
