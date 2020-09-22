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
#include "mainwindow.h"
#include "constants.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QScrollBar>
#include <QScreen>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>

#include <DFontSizeManager>
#include <DGuiApplicationHelper>
#include <DRegionMonitor>

#define DOCK_TOP        0
#define DOCK_RIGHT      1
#define DOCK_BOTTOM     2
#define DOCK_LEFT       3

MainWindow::MainWindow(QWidget *parent)
    : DBlurEffectWidget(parent)
    , m_displayInter(new DBusDisplay("com.deepin.daemon.Display", "/com/deepin/daemon/Display", QDBusConnection::sessionBus(), this))
    , m_daemonDockInter(new DBusDaemonDock("com.deepin.dde.daemon.Dock", "/com/deepin/dde/daemon/Dock", QDBusConnection::sessionBus(), this))
    , m_dockInter(new DBusDockInterface)
    , m_content(new DWidget(parent))
    , m_listview(new ListView(this))
    , m_model(new ClipboardModel(m_listview))
    , m_itemDelegate(new ItemDelegate)
    , m_xAni(new QPropertyAnimation(this))
    , m_widthAni(new QPropertyAnimation(this))
    , m_aniGroup(new QSequentialAnimationGroup(this))
    , m_wmHelper(DWindowManagerHelper::instance())
{
    initUI();
    initAni();
    initConnect();

    geometryChanged();
    CompositeChanged();

    installEventFilter(this);

    checkXEventMonitorDbusState();
}

MainWindow::~MainWindow()
{

}

void MainWindow::Toggle()
{
    if (m_aniGroup->state() == QAbstractAnimation::Running)
        return;

    if (isVisible()) {
        hideAni();
    } else {
        showAni();
    }
}

void MainWindow::geometryChanged()
{
    adjustPosition();

    setX(WindowMargin);

    //init animation by 'm_rect'
    m_xAni->setStartValue(WindowMargin);
    m_xAni->setEndValue(0);

    m_widthAni->setStartValue(m_rect.width());
    m_widthAni->setEndValue(0);
}

void MainWindow::showAni()
{
    if (!m_hasComposite) {
        move(m_rect.x() + WindowMargin, m_rect.y());
        setFixedWidth(m_rect.width());
        show();
        return;
    }

    move(m_rect.x(), m_rect.y());
    setFixedWidth(0);

    show();
    m_aniGroup->setDirection(QAbstractAnimation::Backward);
    m_aniGroup->start();
}

void MainWindow::hideAni()
{
    if (!m_hasComposite) {
        hide();
        return;
    }
    m_aniGroup->setDirection(QAbstractAnimation::Forward);
    m_aniGroup->start();

    QTimer::singleShot(m_aniGroup->duration(), this, [ = ] {setVisible(false);});
}

void MainWindow::startLoader()
{
    QProcess *process = new QProcess(this);
    process->startDetached("dde-clipboardloader");
    process->waitForStarted();
    process->waitForFinished();
    process->deleteLater();
    process = Q_NULLPTR;
}

void MainWindow::setX(int x)
{
    move(m_rect.x() + x, m_rect.y());
}

void MainWindow::CompositeChanged()
{
    m_hasComposite = m_wmHelper->hasComposite();
}

void MainWindow::checkXEventMonitorDbusState()
{
    QTimer *timer = new QTimer(this);
    timer->setInterval(100);
    connect(timer, &QTimer::timeout, this, [ = ] {
        // DRegionMonitor依赖以下服务，确保其启动再绑定其信号
        QDBusInterface interface("com.deepin.api.XEventMonitor", "/com/deepin/api/XEventMonitor",
                                     "com.deepin.api.XEventMonitor",
                                     QDBusConnection::sessionBus());
        if (interface.isValid())
        {
            DRegionMonitor *monitor = new DRegionMonitor(this);
            monitor->registerRegion(QRegion(QRect()));
            connect(monitor, &DRegionMonitor::buttonPress, this, [ = ](const QPoint & p, const int flag) {
                Q_UNUSED(flag);
                if (!geometry().contains(p))
                    if (!isHidden()) {
                        hideAni();
                    }
            });
            timer->stop();
        }
    });
    timer->start();
}

void MainWindow::initUI()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool  | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 10);
    mainLayout->setSpacing(0);

    QWidget *titleWidget = new QWidget;
    QHBoxLayout *titleLayout = new QHBoxLayout(titleWidget);
    titleLayout->setContentsMargins(20, 0, 10, 0);

    QLabel *titleLabel = new QLabel(tr("Clipboard"));
    titleLabel->setFont(DFontSizeManager::instance()->t3());

    m_clearButton = new IconButton(tr("Clear all"));
    connect(m_clearButton, &IconButton::clicked, m_model, &ClipboardModel::clear);

    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(m_clearButton);
    m_clearButton->setFixedSize(100, 36);
    m_clearButton->setBackOpacity(200);
    m_clearButton->setRadius(8);
    m_clearButton->setVisible(false);
    titleWidget->setFixedSize(WindowWidth, WindowTitleHeight);

    m_listview->setModel(m_model);
    m_listview->setItemDelegate(m_itemDelegate);
    m_listview->setFixedWidth(WindowWidth);//需固定，否则动画会变形

    mainLayout->addWidget(titleWidget);
    mainLayout->addWidget(m_listview);

    m_content->setLayout(mainLayout);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(m_content);

    setFocusPolicy(Qt::NoFocus);
}

void MainWindow::initAni()
{
    m_xAni->setEasingCurve(QEasingCurve::Linear);
    m_xAni->setPropertyName("x");
    m_xAni->setTargetObject(this);
    m_xAni->setDuration(AnimationTime / 2);

    m_widthAni->setEasingCurve(QEasingCurve::Linear);
    m_widthAni->setPropertyName("width");
    m_widthAni->setTargetObject(this);
    m_widthAni->setDuration(AnimationTime);

    m_aniGroup->addAnimation(m_xAni);
    m_aniGroup->addAnimation(m_widthAni);
}

void MainWindow::initConnect()
{
    connect(m_displayInter, &DBusDisplay::PrimaryRectChanged, this, &MainWindow::geometryChanged, Qt::QueuedConnection);

    connect(m_model, &ClipboardModel::dataChanged, this, [ = ] {
        m_clearButton->setVisible(m_model->data().size() != 0);
    });

    connect(m_model, &ClipboardModel::dataReborn, this, [ = ] {
        hideAni();
    });

    connect(m_dockInter, &DBusDockInterface::geometryChanged, this, &MainWindow::geometryChanged, Qt::UniqueConnection);

    connect(m_wmHelper, &DWindowManagerHelper::hasCompositeChanged, this, &MainWindow::CompositeChanged, Qt::QueuedConnection);

    connect(m_widthAni, &QVariantAnimation::valueChanged, this, [ = ](QVariant value) {
        int width = value.toInt();
        m_content->move(width - 300, m_content->pos().y());
    });
}

void MainWindow::adjustPosition()
{
    // 屏幕尺寸
    QRect rect = getDisplayScreen();
    qreal scale = qApp->primaryScreen()->devicePixelRatio();
    rect.setWidth(WindowWidth);
    rect.setHeight(int(std::round(qreal(rect.height()) / scale)));

    QRect dockRect = m_dockInter->geometry();
    dockRect.setWidth(int(std::round(qreal(dockRect.width()) / scale)));
    dockRect.setHeight(int(std::round(qreal(dockRect.height()) / scale)));

    // 初始化剪切板位置
    switch (m_daemonDockInter->position()) {
    case DOCK_TOP:
        rect.moveTop(dockRect.height());
        rect.setHeight(rect.height() - dockRect.height());
        break;
    case DOCK_BOTTOM:
        rect.setHeight(rect.height() - dockRect.height());
        break;
    case DOCK_LEFT:
        rect.moveLeft(dockRect.width());
        break;
    default:;
    }

    // 左上下部分预留的间隙
    rect -= QMargins(0, WindowMargin, 0, WindowMargin);

    // 针对时尚模式的特殊处理
    // 只有任务栏显示的时候, 才额外偏移
    if(m_daemonDockInter->displayMode() == 0 && dockRect.width() * dockRect.height() > 0) {
        switch (m_daemonDockInter->position()) {
        case DOCK_TOP:
            rect -= QMargins(0, WindowMargin, 0, 0);
            break;
        case DOCK_BOTTOM:
            rect -= QMargins(0, 0, 0, WindowMargin);
            break;
        case DOCK_LEFT:
            rect -= QMargins(WindowMargin, 0, 0, 0);
            break;
        default:;
        }
    }

    setGeometry(rect);
    m_rect = rect;
    setFixedSize(rect.size());
    m_content->setFixedSize(rect.size());
}

QRect MainWindow::getDisplayScreen()
{
    QRect dockRect = m_dockInter->geometry();
    for (const auto& monitorPath : m_displayInter->monitors()) {
        DisplayMonitor monitor("com.deepin.daemon.Display", monitorPath.path(), QDBusConnection::sessionBus());
        QRect screenRect(monitor.x(), monitor.y(), monitor.width(), monitor.height());
        if (screenRect.contains(dockRect))
            return screenRect;
    }
    return m_displayInter->primaryRect();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    //禁止窗口被拖动
    Q_UNUSED(event);
    return;
}

