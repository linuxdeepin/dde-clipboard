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
    if (!interface.isValid()) {
        qDebug() << "start loader...";
        w.startLoader();
    }

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
