// Copyright (C) 2020 ~ 2022 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "clipboarddaemon.h"
#include "clipboardloader.h"

#include <QDBusError>
#include <QDBusConnection>
#include <QGuiApplication>

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
}
