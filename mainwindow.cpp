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
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool  | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowStaysOnTopHint);

    initUI();
    initConnect();

    geometryChanged();
}

MainWindow::~MainWindow()
{

}

void MainWindow::Toggle()
{
    setVisible(windowState() == Qt::WindowMinimized || !isVisible());

    if (isVisible()) {
        //显示后，在桌面Super+D快捷键，再点击桌面空白处，此时无法show出，需active
        activateWindow();
    }
}

void MainWindow::initUI()
{
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

    mainLayout->addWidget(titleWidget);
    mainLayout->addWidget(m_listview);
    setLayout(mainLayout);
}

void MainWindow::initConnect()
{
    connect(m_displayInter, &DBusDisplay::PrimaryRectChanged, this, &MainWindow::geometryChanged, Qt::QueuedConnection);

    connect(m_model, &ClipboardModel::dataChanged, this, [ = ] {
        m_clearButton->setVisible(m_model->data().size() != 0);
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
    rect.setWidth(scale * (WindowWidth + WindowMargin * 2));
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
    setFixedSize(rect.size());
}

