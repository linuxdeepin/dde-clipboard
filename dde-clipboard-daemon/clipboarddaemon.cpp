/*
 * Copyright (C) 2020 ~ 2022 Deepin Technology Co., Ltd.
 *
 * Author:     zhaoyingzhen <zhaoyingzhen@uniontech.com>
 *
 * Maintainer: zhaoyingzhen <zhaoyingzhen@uniontech.com>
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

#include "clipboarddaemon.h"
#include "clipboardloader.h"

#include <QDBusError>
#include <QDBusConnection>

ClipboardDaemon::ClipboardDaemon(QObject *parent)
    : QObject(parent)
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerService("org.deepin.dde.ClipboardLoader1")) {
        qInfo() << "error:" << connection.lastError().message();
    }

    ClipboardLoader *clipboardLoader = new ClipboardLoader(this);
    connection.registerObject("/org/deepin/dde/ClipboardLoader1", clipboardLoader,
                              QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);

    // 剪切板管理和提供UI数据的功能合并，WaylandCopyClient只实例化一次
#if 0
    // 实例化wayland 剪切板管理器
    if (qEnvironmentVariable("XDG_SESSION_TYPE").contains("wayland")) {
        WaylandCopyClient *waylandClipboardManager = new WaylandCopyClient(this);
        waylandClipboardManager->init(true);
    }
#endif
}
