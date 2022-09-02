// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>
#include <QDBusError>
#include <QDBusConnection>

#include <DLog>

#include "clipboarddaemon.h"

DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName("deepin");
    a.setApplicationName("dde-clipboard-daemon");

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    const QString interface = "com.deepin.daemon.Clipboard";
    const QString path = "/com/deepin/daemon/Clipboard";
    if (!QDBusConnection::sessionBus().registerService(interface)) {
        qWarning() << "DBus register failed, error message:" << QDBusConnection::sessionBus().lastError().message();
        exit(-1);
    }

    ClipboardDaemon daemon;
    QDBusConnection::sessionBus().registerObject(path, &daemon, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);
    qDebug() << "Everything is ok!";

    return a.exec();
}