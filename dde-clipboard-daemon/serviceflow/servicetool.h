// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SERVICETOOL
#define SERVICETOOL

#include "commandmessage.h"

#include <QList>
#include <QWaitCondition>
#include <QMutex>

class Transporter
{
public:
    Transporter();
    ~Transporter();

    void putMessage(CommandMessage *msg);
    CommandMessage *takeMessage();
    CommandMessage *firstMessage() const;

    int messageCount() const;
    void release();

private:
    QList<CommandMessage *> m_messages;
    QWaitCondition m_condition;
    QMutex m_deleteMutex;
    QMutex m_mutex;
    bool m_shouldDelete;
};

class Command;
class Extractor
{
    friend class Command;
public:
    enum Direction {
        TransporterToCommand,
        CommandToTransporter
    };

    Extractor(Transporter *trans);
    Extractor(Transporter *trans, Direction dir);

    void setDirection(Direction dir);
    Direction direction() const;

protected:
    CommandMessage *takeFromTransporter();
    void pushToTransporter(CommandMessage *msg);

    void appendCommand(Command *command);

private:
    Direction m_direction;
    QList<Command *> m_commands;
    Transporter *m_transporter;
};

#endif  // SERVICETOOL
