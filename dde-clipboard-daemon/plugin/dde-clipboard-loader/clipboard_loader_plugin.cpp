/*
 * Copyright (C) 2019 ~ 2022 Uniontech Technology Co., Ltd.
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
#include "clipboard_loader_plugin.h"

#include <QString>
#include <QProcess>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusError>
#include <QDeferredDeleteEvent>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>

#include "clipboard_loader.h"

static ClipboardLoader *worker = nullptr;
static UnloadFun unloadFun = nullptr;
static QByteArray info;
bool Start()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.registerService("com.deepin.dde.ClipboardLoader")) {
        qDebug() << "error:" << connection.lastError().message();
        return false;
    }

    if (!worker) {
        worker = new ClipboardLoader;
    }

    connection.registerObject("/com/deepin/dde/ClipboardLoader", worker,
                              QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals);
    return true;
}

bool Stop()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    if (!connection.unregisterService("com.deepin.dde.ClipboardLoader")) {
        qDebug() << "error:" << connection.lastError().message();
        return false;
    }

    if (worker) {
        QDeferredDeleteEvent *event = new QDeferredDeleteEvent;
        qApp->postEvent(worker, event);
        worker = nullptr;
    }

    if (unloadFun) {
        unloadFun = nullptr;
    }

    return true;
}

const char *Info()
{
    if (info.isEmpty()) {
        QVariantHash data;
        data.insert("name","clipboard-loader");
        data.insert("version","1.0");
        data.insert("service","");
        data.insert("enabled", true);
        QJsonObject rootObj = QJsonObject::fromVariantHash(data);
        QJsonDocument document;
        document.setObject(rootObj);
        info = QString(document.toJson()).toLocal8Bit();
    }

    return info.data();
}

void UnloadCallBack(UnloadFun fun)
{
    unloadFun = fun;
}
