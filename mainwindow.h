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
class QParallelAnimationGroup;
class MainWindow : public DBlurEffectWidget
{
    Q_OBJECT
    Q_PROPERTY(int width WRITE setFixedWidth)
    Q_PROPERTY(int x WRITE setX)
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

public Q_SLOTS:
    void Toggle();
    void showAni();
    void hideAni();

private Q_SLOTS:
    void geometryChanged();
    void setX(int x);
    void CompositeChanged();

private:
    void initUI();
    void initAni();
    void initConnect();

protected:
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
    QParallelAnimationGroup *m_aniGroup;

    QRect m_rect;

    DWindowManagerHelper *m_wmHelper;

    bool m_hasComposite = false;
};

#endif // MAINWINDOW_H
