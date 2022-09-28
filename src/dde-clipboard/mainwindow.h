// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QListView>

#include "clipboardmodel.h"
#include "itemdelegate.h"
#include "constants.h"
#include "dbusdockinterface.h"
#include "listview.h"
#include "iconbutton.h"

#include <DBlurEffectWidget>
#include <DWindowManagerHelper>
#include <DRegionMonitor>

#include <com_deepin_daemon_display_monitor.h>
#include <com_deepin_dde_daemon_dock.h>
#include <com_deepin_daemon_display.h>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

class QPushButton;
class QPropertyAnimation;
class QSequentialAnimationGroup;

using DBusDisplay = com::deepin::daemon::Display;
using DisplayMonitor = com::deepin::daemon::display::Monitor;
using DBusDaemonDock = com::deepin::dde::daemon::Dock;

/*!
 * \~chinese \class MainWindow
 * \~chinese \brief 主窗口类
 */
class MainWindow : public DBlurEffectWidget
{
    Q_OBJECT
    Q_PROPERTY(int width READ getWidth WRITE setFixedWidth)
    Q_PROPERTY(int x READ getX WRITE setX)
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

public Q_SLOTS:
    /*!
     * \~chinese \name Toggle
     * \~chinese \brief 切换主窗口显示或隐藏的状态
     */
    void Toggle();
    /*!
     * \~chinese \name showAni
     * \~chinese \brief 主窗口显示时的动画
     */
    void showAni();
    /*!
     * \~chinese \name hideAni
     * \~chinese \brief 主窗口隐藏时的动画
     */
    void hideAni();

    void startLoader();

    void Show();
    void Hide();

private Q_SLOTS:
    /*!
     * \~chinese \name geometryChanged
     * \~chinese \brief 根据dock和屏幕的大小改变窗口的大小,根据dock的位置调整主窗口的位置
     */
    void geometryChanged();
    /*!
     * \~chinese \name setX
     * \~chinese \brief 设置主窗口X轴的起始坐标
     * \~chinese \param X轴坐标位置
     */
    void setX(int x);
    /*!
     * \~chinese \name CompositeChanged
     * \~chinese \brief 是否开启动画效果
     */
    void CompositeChanged();

    void registerMonitor();

private:
    /*!
     * \~chinese \name initUI
     * \~chinese \brief 初始化界面布局
     */
    void initUI();
    /*!
     * \~chinese \name initAni
     * \~chinese \brief 初始化动画效果
     */
    void initAni();
    /*!
     * \~chinese \name initConnect
     * \~chinese \brief 初始化信号与槽的连接
     */
    void initConnect();
    /*!
     * \~chinese \name adjustPosition
     * \~chinese \brief 调整剪切板位置
     */
    void adjustPosition();
    /*!
     * \~chinese \name getDisplayScreen
     * \~chinese \brief 获取显示屏幕的坐标
     */
    QRect getDisplayScreen();


    int getWidth() const { return this->width(); }
    int getX() const { return this->pos().x(); }
protected:
    /*!
     * \~chinese \name initConnect
     * \~chinese \brief 重写mouseMoveEvent事件禁止窗口被移动
     */
    virtual void mouseMoveEvent(QMouseEvent *event) override;

private:
    DBusDisplay *m_displayInter;
    DBusDaemonDock *m_daemonDockInter;
    DBusDockInterface *m_dockInter;
    DRegionMonitor *m_regionMonitor;

    DWidget *m_content;
    IconButton *m_clearButton;
    ListView *m_listview;
    ClipboardModel *m_model;
    ItemDelegate *m_itemDelegate;

    QPropertyAnimation *m_xAni;
    QPropertyAnimation *m_widthAni;
    QSequentialAnimationGroup *m_aniGroup;

    QRect m_rect;

    DWindowManagerHelper *m_wmHelper;

    bool m_hasComposite = false;
};

#endif // MAINWINDOW_H
