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

#ifdef USE_DEEPIN_WAYLAND
#include "waylandcopyclient.h"

static WaylandCopyClient *manager = nullptr;
#endif // USE_DEEPIN_WAYLAND

static UnloadFun unloadFun = nullptr;
static QByteArray info;
bool Start()
{
#ifdef USE_DEEPIN_WAYLAND
    if (QGuiApplication::platformName().startsWith("wayland", Qt::CaseInsensitive)) {
        manager = &WaylandCopyClient::ref();
        manager->init();
    }
#endif // USE_DEEPIN_WAYLAND

    return true;
}

bool Stop()
{
#ifdef USE_DEEPIN_WAYLAND
    if (manager) {
        QDeferredDeleteEvent *event = new QDeferredDeleteEvent;
        qApp->postEvent(manager, event);
        manager = nullptr;
    }
#endif // USE_DEEPIN_WAYLAND

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
