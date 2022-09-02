// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "clipboarddaemon.h"
#include "clipboardloader.h"

#include <QDBusError>
#include <QDBusConnection>

ClipboardDaemon::ClipboardDaemon(QObject *parent)
    : QObject(parent)
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerService("com.deepin.dde.ClipboardLoader")) {
        qInfo() << "error:" << connection.lastError().message();
    }

    ClipboardLoader *clipboardLoader = new ClipboardLoader(this);
    connection.registerObject("/com/deepin/dde/ClipboardLoader", clipboardLoader,
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
