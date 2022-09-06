// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DBUSMANAGER_H
#define DBUSMANAGER_H

#include <QObject>
#include <QMap>
#include <QDBusContext>

class QLibrary;
class DBusManager : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.dde.Clipboard")
    Q_CLASSINFO("D-Bus Path", "/com/deepin/dde/Clipboard")

public:
    static DBusManager *instance( QObject *parent = nullptr);

public Q_SLOTS:
    void Load(const QString &fileName);
    void UnLoad(const QString &name);
    bool IsRunning(const QString &name);
    QStringList PluginList();
    void LoadAllPlugins();

Q_SIGNALS:
    void PluginLoaded(const QString &name);
    void PluginUnLoaded(const QString &name);

private:
    DBusManager( QObject *parent = nullptr);
    ~ DBusManager();
    struct PluginInfo{
        QString name;
        QString version;
        QString service;
        bool enabled = true;
    };

    const QString getPluginPath();
    void resolveInfo(QByteArray data, PluginInfo &info);

private:
    static DBusManager *m_instance;
    QMap<QString /*name*/, QPair<QString/*fileName*/, QLibrary * /*lib*/>> m_map;
};

#endif // DBUSMANAGER_H
