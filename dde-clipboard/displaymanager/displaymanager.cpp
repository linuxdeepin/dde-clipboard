/*
 * Copyright (C) 2018 ~ 2021 Uniontech Technology Co., Ltd.
 *
 * Author:     fanpengcheng <fanpengcheng@uniontech.com>
 *
 * Maintainer: fanpengcheng <fanpengcheng@uniontech.com>
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

#include "displaymanager.h"

#include <QScreen>
#include <QApplication>

DisplayManager *DisplayManager::instance()
{
    static DisplayManager t;
    return &t;
}

DisplayManager::DisplayManager(QObject *parent)
    : QObject(parent)
{
    connect(qApp, &QGuiApplication::screenAdded, this, &DisplayManager::screenCountChanged);
    connect(qApp, &QGuiApplication::screenRemoved, this, &DisplayManager::screenCountChanged);
    screenCountChanged();

    QTimer::singleShot(0, this, &DisplayManager::screenInfoChanged);
}

void DisplayManager::screenCountChanged()
{
    // 找到过期的screen指针
    QList<QScreen *> to_remove_list;
    for (auto s : m_screens) {
        if (!qApp->screens().contains(s))
            to_remove_list.append(s);
    }

    // 找出新增的screen指针
    QList<QScreen *> to_add_list;
    for (auto s : qApp->screens()) {
        if (!m_screens.contains(s)) {
            to_add_list.append(s);
        }
    }

    // 取消关联
    for (auto s : to_remove_list) {
        disconnect(s);
        m_screens.removeOne(s);
    }

    // 创建关联
    for (auto s : to_add_list) {
        s->setOrientationUpdateMask(Qt::PrimaryOrientation
                                    | Qt::LandscapeOrientation
                                    | Qt::PortraitOrientation
                                    | Qt::InvertedLandscapeOrientation
                                    | Qt::InvertedPortraitOrientation);

        connect(s, &QScreen::geometryChanged, this, &DisplayManager::screenInfoChanged);
        connect(s, &QScreen::availableGeometryChanged, this, &DisplayManager::screenInfoChanged);
        connect(s, &QScreen::physicalSizeChanged, this, &DisplayManager::screenInfoChanged);
        connect(s, &QScreen::physicalDotsPerInchChanged, this, &DisplayManager::screenInfoChanged);
        connect(s, &QScreen::logicalDotsPerInchChanged, this, &DisplayManager::screenInfoChanged);
        connect(s, &QScreen::virtualGeometryChanged, this, &DisplayManager::screenInfoChanged);
        connect(s, &QScreen::primaryOrientationChanged, this, &DisplayManager::screenInfoChanged);
        connect(s, &QScreen::orientationChanged, this, &DisplayManager::screenInfoChanged);
        connect(s, &QScreen::refreshRateChanged, this, &DisplayManager::screenInfoChanged);

        m_screens.append(s);
    }

    Q_EMIT screenInfoChanged();
}
