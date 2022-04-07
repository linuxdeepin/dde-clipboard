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
#include "dbus_manager.h"

#include <QLibrary>
#include <QDebug>
#include <QDir>
#include <QTimer>
#include <QLoggingCategory>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDBusServiceWatcher>
#include <QDBusConnection>
#include <QDBusConnectionInterface>

Q_LOGGING_CATEGORY(plugin, "===dde.plugin")

typedef const char *(*InfoFun)();
typedef void (*StartFun)();
typedef void (*StopFun)();
typedef void (*UnloadFun)(const char *);
typedef void (*UnloadCallBack)(UnloadFun);

void OnUnload(const char *name) {
    qCDebug(plugin) << name << "request unload self";
    DBusManager::instance()->UnLoad(name);
}

static const QStringList CompatiblePluginApiList {
    "1.0",
};

DBusManager *DBusManager::m_instance = nullptr;
DBusManager::DBusManager(QObject *parent)
    : QObject(parent)
{

}

DBusManager::~DBusManager()
{
    foreach(auto pair, m_map.values()) {
        auto lib = pair.second;
        if (lib->isLoaded())
            lib->unload();
        lib->deleteLater();
    }

    m_map.clear();
}

DBusManager *DBusManager::instance( QObject *parent)
{
    if (!m_instance)
        m_instance = new DBusManager(parent);
    return m_instance;
}

void DBusManager::LoadAllPlugins()
{
    // 启动时加载所有插件
    QDir libDir(getPluginPath());
    qInfo() << "plugins dir: " << libDir.absolutePath();
    for (auto name : libDir.entryList(QStringList() << "*")) {
        Load(name);
    }
}

/**
 * @brief DBusManager::Load     手动加载指定插件
 * @param pluginName            插件的文件名，不需要包含路径
 * @return                      是否加载成功
 * @note                        插件需要放置在loader进程父目录的lib文件夹下
 */
void DBusManager::Load(const QString &fileName)
{
    QFileInfo fileInfo(getPluginPath() + QDir::separator() + fileName);
    if (!QLibrary::isLibrary(fileInfo.absoluteFilePath()))
        return;

    QLibrary *lib = new QLibrary(fileInfo.absoluteFilePath());

    InfoFun infoFun = InfoFun(lib->resolve("Info"));
    if (!infoFun) {
        qCDebug(plugin) << "failed to resolve the `Info` method: "<< fileInfo.fileName() ;
        if (lib->isLoaded())
            lib->unload();
        lib->deleteLater();
        return;
    }

    PluginInfo info;
    const char *data = infoFun();
    auto ba = QByteArray(data);
    if (data) {
        resolveInfo(ba, info);
    }

    if (info.name.isEmpty() || !info.enabled) {
        qCDebug(plugin) << "unload plugin , name: " << info.name << ", enabled: " << info.enabled;
        if (lib->isLoaded())
            lib->unload();
        lib->deleteLater();
        return;
    }

    if (info.version.isEmpty() || !CompatiblePluginApiList.contains(info.version)) {
        qCDebug(plugin) << "plugin api version not matched! expect versions:" << CompatiblePluginApiList
                 << ", got version:" << info.version
                 << ", the plugin file is:" << fileInfo.absoluteFilePath();
        if (lib->isLoaded())
            lib->unload();
        lib->deleteLater();
        return;
    }

    /**
     * @brief initPlugin                    调用插件的Start接口正式启动插件
     * @param fileName                      插件的文件名
     * @param name                          插件名，通过Info接口解析得到
     * @param lib                           解析插件的指针
     */
    auto initPlugin = [ = ] (const QString &fileName, const QString &name, QLibrary *lib) {
        if (name.isEmpty() || !lib)
            return;

        if (m_map.keys().contains(name)) {
            qCDebug(plugin) << name << ": this plugin is already loaded.";
            if (lib->isLoaded())
                lib->unload();
            lib->deleteLater();
            return;
        }

        StartFun startFun = StartFun(lib->resolve("Start"));
        if (!startFun) {
            qCDebug(plugin) << fileName << ": failed to resolve the `Start` method";
            if (lib->isLoaded())
                lib->unload();
            lib->deleteLater();
            return;
        }

        // 允许插件主动退出
        UnloadCallBack unloadCallBack = UnloadCallBack(lib->resolve("UnloadCallBack"));
        if (unloadCallBack) {
            unloadCallBack(&OnUnload);
        }

        qDebug() << "load: " << name;
        startFun();
        qDebug() << "load finished: " << name;

        QPair <QString, QLibrary *> pair;
        pair.first = fileName;
        pair.second = lib;
        m_map.insert(name, pair);

        QMetaObject::invokeMethod(this, "PluginLoaded", Qt::QueuedConnection, Q_ARG(QString, name));
    };

    // 存在依赖的服务,等待服务启动后再执行初始化，只会在第一次启动后初始化
     if(!info.service.isEmpty()) {
        if (QDBusConnection::sessionBus().interface()->isServiceRegistered(info.service)) {
            initPlugin(fileInfo.fileName(), info.name, lib);
        } else {
            qCDebug(plugin) << "session service " << info.service << " not start, wait...";
            QDBusServiceWatcher *watcher = new QDBusServiceWatcher(info.service, QDBusConnection::sessionBus(), QDBusServiceWatcher::WatchForOwnerChange, this);
            connect(watcher, &QDBusServiceWatcher::serviceRegistered, this, [ = ] {
                qCDebug(plugin) << "session service " << info.service << " started";
                initPlugin(fileInfo.fileName(), info.name, lib);
                watcher->deleteLater();
            });
        }
    } else {
        initPlugin(fileInfo.fileName(), info.name, lib);
    }
}

/**
 * @brief DBusManager::UnLoad       手动卸载指定插件
 * @param name                      插件名，通过解析Info接口中的name字段得到
 * @return                          是否卸载成功
 * @note                            插件需要放置在loader进程父目录的lib文件夹下
 */
void DBusManager::UnLoad(const QString &name)
{
    if (!m_map.keys().contains(name)) {
        if (calledFromDBus())
            sendErrorReply(QDBusError::Failed, "This plugin is not loaded.");
        return;
    }

    QLibrary *lib = m_map.value(name).second;
    if (!lib) {
        if (calledFromDBus())
            sendErrorReply(QDBusError::Failed, "This plugin is not loaded.");
        return;
    }

    StopFun stopFun = StopFun(lib->resolve("Stop"));
    if (!stopFun) {
        if (calledFromDBus())
            sendErrorReply(QDBusError::Failed, "Failed to resolve the `Stop` method");
        return;
    }

    qCDebug(plugin) << "unload: " << name;
    stopFun();
    qCDebug(plugin) << "unload finished: " << name;

    if (lib->isLoaded())
        lib->unload();
    lib->deleteLater();

    m_map.remove(name);

    Q_EMIT PluginUnLoaded(name);
}

/**
 * @brief DBusManager::IsRunning        判断插件当前是否正在运行
 * @param name                          插件名，通过解析Info接口中的name字段得到
 * @return                              是否正在运行
 * @note                                插件未提供指定接口的会拒绝加载
 */
bool DBusManager::IsRunning(const QString &name)
{
    return m_map.value(name).second;
}

/**
 * @brief DBusManager::PluginList       所有加载中的插件
 * @return                              导出的为插件的name的集合
 */
QStringList DBusManager::PluginList()
{
    return m_map.keys();
}

/**
 * @brief DBusManager::getPluginPath    获取插件存放路径
 * @param type                          系统级插件还是用户级插件目录
 * @return
 */
const QString DBusManager::getPluginPath()
{
#ifdef QT_DEBUG
    QDir dir = QDir::currentPath();
    dir.cd("plugins");
#else
    QDir dir("/usr/lib/dde-clipboard");
#endif
    return dir.absolutePath();
}

/**
 * @brief DBusManager::resolveInfo      解析插件的信息
 * @param data                          通过插件的Info接口获取的数据
 * @param info                          结构体引用，解析后的数据会存放到结构体中
 */
void DBusManager::resolveInfo(QByteArray data, DBusManager::PluginInfo &info)
{
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(data, &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        qCDebug(plugin) << "plugin info parse failed!!!";
        return;
    }

    QJsonObject obj = doc.object();
    info.name = obj.value("name").toString();
    info.version = obj.value("version").toString();
    info.service = obj.value("service").toString();
    info.enabled = obj.value("enabled").toBool();
}
