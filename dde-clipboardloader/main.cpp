/*
 * Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     fanpengcheng_cm <fanpengcheng_cm@deepin.com>
 *
 * Maintainer: fanpengcheng_cm <fanpengcheng_cm@deepin.com>
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
#include <DApplication>
#include <DWidgetUtil>
#include <DLog>

#include <QProcess>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusError>

#include "clipboardloader.h"

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    DApplication::loadDXcbPlugin();
    DApplication a(argc, argv);
    a.setAttribute(Qt::AA_UseHighDpiPixmaps);
    a.setOrganizationName("deepin");
    a.setApplicationName("dde-clipboardloader");
    a.setApplicationVersion("1.0");
    a.setSingleInstance("dde-clipboardloader");

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerService("com.deepin.dde.ClipboardLoader")) {
        qDebug() << "error:" << connection.lastError().message();
        exit(-1);
    }
    ClipboardLoader worker;
    connection.registerObject("/com/deepin/dde/ClipboardLoader", &worker,
                              QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);

    return a.exec();
}
