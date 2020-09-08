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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QListView>

#include "clipboardmodel.h"
#include "dbusdisplay.h"
#include "itemdelegate.h"
#include "constants.h"
#include "dbusdock.h"
#include "listview.h"
#include "iconbutton.h"

#include <DBlurEffectWidget>
#include <DWindowManagerHelper>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

class QPushButton;
class QPropertyAnimation;
class QSequentialAnimationGroup;
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
    MainWindow(QWidget *parent = nullptr);
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
    void checkXEventMonitorDbusState();

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
    DBusDock *m_dockInter;

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
