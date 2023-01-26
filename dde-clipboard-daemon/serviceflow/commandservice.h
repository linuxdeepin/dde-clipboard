// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMANDSERVICE
#define COMMANDSERVICE

#include "command.h"

class CommandService
{
public:
    CommandService();
    virtual ~CommandService();

    virtual QList<Command *> commands() = 0;

    CommandService *nextService() const;
    void setNextService(CommandService* service);

private:
    CommandService* m_nextService;
};

#endif  // COMMANDSERVICE
