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

#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include <QObject>

#include "constants.h"

class QScreen;
class DisplayManager: public QObject
{
    Q_OBJECT
public:
    static DisplayManager *instance();

private:
    explicit DisplayManager(QObject *parent = Q_NULLPTR);

private Q_SLOTS:
    void screenCountChanged();

Q_SIGNALS:
    void screenInfoChanged();

private:
    QList<QScreen *> m_screens;
};

#endif // DISPLAYMANAGER_H
