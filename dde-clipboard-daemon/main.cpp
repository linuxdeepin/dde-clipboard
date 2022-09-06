// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <QApplication>
#include <QDBusConnection>
#include <QDebug>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QMetaClassInfo>

#include <DLog>

#include "dbus_manager.h"

DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName("deepin");
    a.setApplicationName("dde-clipboard-daemon");
    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    DBusManager *manager = DBusManager::instance();
    const QString &interface = "com.deepin.daemon.Clipboard";
    const QString &path = "/com/deepin/daemon/Clipboard";

    if (!QDBusConnection::sessionBus().registerService(interface)) {
        qDebug() << "DBus register failed, error message:" << QDBusConnection::sessionBus().lastError().message();
        exit(-1);
    }
    QDBusConnection::sessionBus().registerObject(path, manager, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);
    manager->LoadAllPlugins();
    qDebug() << "Everything is ok!";
    return a.exec();
}
