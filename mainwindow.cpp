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

#include <DFontSizeManager>
#include <DGuiApplicationHelper>

#define DOCK_TOP        0
#define DOCK_RIGHT      1
#define DOCK_BOTTOM     2
#define DOCK_LEFT       3

MainWindow::MainWindow(QWidget *parent)
    : DBlurEffectWidget(parent)
    , m_displayInter(new DBusDisplay(this))
    , m_listview(new ListView(this))
    , m_model(new ClipboardModel(m_listview))
    , m_itemDelegate(new ItemDelegate)
    , m_dockInter(new DBusDock)
    , m_inAni(new QPropertyAnimation(this))
    , m_outAni(new QPropertyAnimation(this))
    , m_start(false)
{
    initUI();
    initAni();
    initConnect();

    geometryChanged();
}

MainWindow::~MainWindow()
{

}

void MainWindow::Toggle()
{
    if(m_aniTickTime.elapsed() < AnimationTime && m_start)
        return;

    m_start = true;
    m_aniTickTime.restart();

    if (isVisible()) {
        hideAni();
    }
    else {
        showAni();
    }
}

void MainWindow::initUI()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool  | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowStaysOnTopHint);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // title
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
    setLayout(mainLayout);
}

void MainWindow::initAni()
{
    m_inAni->setEasingCurve(QEasingCurve::OutCirc);
    m_inAni->setPropertyName("width");
    m_inAni->setTargetObject(this);
    m_inAni->setDuration(AnimationTime);

    m_outAni->setEasingCurve(QEasingCurve::OutCubic);
    m_outAni->setPropertyName("width");
    m_outAni->setTargetObject(this);
    m_outAni->setDuration(AnimationTime);

    m_aniTickTime.start();
}

void MainWindow::initConnect()
{
    connect(m_displayInter, &DBusDisplay::PrimaryRectChanged, this, &MainWindow::geometryChanged, Qt::QueuedConnection);

    connect(m_model, &ClipboardModel::dataChanged, this, [ = ] {
        m_clearButton->setVisible(m_model->data().size() != 0);
    });

    connect(m_model, &ClipboardModel::dataComing, this, [ = ] {
        m_listview->scrollToTop();
    });

    connect(m_dockInter, &DBusDock::FrontendRectChanged, this, &MainWindow::geometryChanged, Qt::UniqueConnection);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    return;
}

void MainWindow::geometryChanged()
{
    // 屏幕尺寸
    QRect rect = m_displayInter->primaryRawRect();
    qreal scale = qApp->primaryScreen()->devicePixelRatio();
    rect.setWidth(WindowWidth + WindowMargin * 2);
    rect.setHeight(std::round(qreal(rect.height()) / scale));

    QRect dockRect = m_dockInter->frontendRect();
    dockRect.setWidth(std::round(qreal(dockRect.width()) / scale));
    dockRect.setHeight(std::round(qreal(dockRect.height()) / scale));

    switch (m_dockInter->position()) {
    case DOCK_TOP:
        rect.moveTop(dockRect.height());
        [[clang::fallthrough]];
    case DOCK_BOTTOM:
        rect.setHeight(rect.height() - dockRect.height());
        break;
    case DOCK_LEFT:
        rect.moveLeft(dockRect.width());
        break;
    default:;
    }

    rect = rect.marginsRemoved(QMargins(WindowMargin, WindowMargin, WindowMargin, WindowMargin));
    setGeometry(rect);
    m_rect = rect;
    setFixedSize(rect.size());
}

void MainWindow::showAni()
{
    show();

    m_inAni->setStartValue(this->width());
    m_inAni->setEndValue(m_rect.width());
    m_inAni->start();
}

void MainWindow::hideAni()
{
    m_outAni->setStartValue(this->width());
    m_outAni->setEndValue(0);
    m_outAni->start();

    QTimer::singleShot(AnimationTime,[=]{setVisible(false);});
}
