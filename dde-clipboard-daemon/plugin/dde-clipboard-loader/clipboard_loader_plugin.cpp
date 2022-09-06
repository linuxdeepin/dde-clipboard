// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
