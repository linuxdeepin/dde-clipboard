// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"
#include "constants.h"
#include "clipboard_adaptor.h"

#include <DApplication>
#include <DGuiApplicationHelper>
#include <DLog>
#include <unistd.h>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE
DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    DGuiApplicationHelper::setAttribute(DGuiApplicationHelper::UseInactiveColorGroup, false);
    DGuiApplicationHelper::setAttribute(DGuiApplicationHelper::ColorCompositing, true);

    DApplication *app = DApplication::globalApplication(argc, argv);

    app->setOrganizationName("deepin");
    app->setApplicationName("dde-clipboard");
    app->setApplicationDisplayName("DDE Clipboard");
    app->setApplicationVersion("1.0");
    app->loadTranslator();

    if (!DGuiApplicationHelper::setSingleInstance(QString("dde-clipboard_%1").arg(getuid()))) {
        qDebug() << "set single instance failed!";
        return -1;
    }

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    QDBusConnection connection = QDBusConnection::sessionBus();

    MainWindow w;

    QDBusInterface interface("org.deepin.dde.ClipboardLoader1", "/org/deepin/dde/ClipboardLoader1",
                                 "org.deepin.dde.ClipboardLoader1",
                                 QDBusConnection::sessionBus());

    ClipboardAdaptor adapt(&w);
    if (!connection.registerService(DBusClipBoardService)) {
        qDebug() << "error:" << connection.lastError().message();
        exit(-1);
    }

    connection.registerObject(DBusClipBoardPath, &w);

#ifdef QT_DEBUG
    w.showAni();
#endif

    return app->exec();
}
