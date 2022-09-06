// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "clipboard_manager_plugin.h"

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

#include "waylandcopyclient.h"
#ifdef USE_DEEPIN_KF5_WAYLAND
static WaylandCopyClient *manager = nullptr;
#endif
static UnloadFun unloadFun = nullptr;
static QByteArray info;
bool Start()
{
    if (QGuiApplication::platformName().startsWith("wayland", Qt::CaseInsensitive)) {
#ifdef USE_DEEPIN_KF5_WAYLAND
        manager = &WaylandCopyClient::ref();
        manager->init();
#else
        qWarning() << "we will not work with wayland";
#endif
    }
    return true;
}

bool Stop()
{
#ifdef USE_DEEPIN_KF5_WAYLAND
    if (manager) {
        QDeferredDeleteEvent *event = new QDeferredDeleteEvent;
        qApp->postEvent(manager, event);
        manager = nullptr;
    }
#endif

    if (unloadFun) {
        unloadFun = nullptr;
    }

    return true;
}

const char *Info()
{
    if (info.isEmpty()) {
        QVariantHash data;
        data.insert("name","clipboard-manager");
        data.insert("version","1.0");
        data.insert("service","");
        data.insert("enabled", QGuiApplication::platformName().startsWith("wayland", Qt::CaseInsensitive));
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
