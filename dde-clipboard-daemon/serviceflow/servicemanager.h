// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMANDSERVICEMANAGER
#define COMMANDSERVICEMANAGER

#include "commandservice.h"

#include <QMap>

class ServiceFlow;
class CommandServiceManager
{
    // ### make singleton
public:
    CommandServiceManager();
    ~CommandServiceManager();

    void registerService(CommandService *root);

    void unregisterService(CommandService *root);
    bool isServiceRegistered(CommandService *service);

    int registeredServicesCount() const;
    int activeServiceFlowsCount() const;

    void setMaxServiceCount(int maxCount);
    int maxServiceCount() const;

    void start(CommandService *service);
    void appendFlowMessage(CommandMessage *message, CommandService *service);
    CommandService *findRootService(CommandService *service);

private:
    QMap<CommandService *, ServiceFlow *> m_rootServices;
    int m_maxServiceCount;
    QList<CommandService *> m_waitingServices;
};

#endif  // COMMANDSERVICEMANAGER
