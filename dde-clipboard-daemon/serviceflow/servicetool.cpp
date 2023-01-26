// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "servicetool.h"

Transporter::Transporter()
    : m_shouldDelete(false)
{

}

Transporter::~Transporter()
{
}

void Transporter::putMessage(CommandMessage *msg)
{
    m_mutex.lock();
    m_messages.append(msg);
    m_condition.wakeAll();
    m_mutex.unlock();
}

CommandMessage *Transporter::takeMessage()
{
    QMutexLocker locker(&m_mutex);
    while (m_messages.isEmpty() && !m_shouldDelete)
        m_condition.wait(&m_mutex);

    CommandMessage *msg = nullptr;
    if (m_messages.length() > 0)
        msg = m_messages.takeFirst();

    return msg;
}

CommandMessage *Transporter::firstMessage() const
{
    return messageCount() > 0 ? m_messages.first() : nullptr;
}

int Transporter::messageCount() const
{
    return m_messages.count();
}

void Transporter::release()
{
    m_mutex.lock();
    m_shouldDelete = true;
    m_condition.wakeAll();
    m_mutex.unlock();
}

Extractor::Extractor(Transporter *trans)
    : m_transporter(trans)
{

}

Extractor::Extractor(Transporter *trans, Direction dir)
    : m_direction(dir)
    , m_transporter(trans)
{

}

void Extractor::setDirection(Direction dir)
{
    m_direction = dir;
}

Extractor::Direction Extractor::direction() const
{
    return m_direction;
}

CommandMessage *Extractor::takeFromTransporter()
{
    if (m_direction != TransporterToCommand)
        return nullptr;
    return m_transporter->takeMessage();
}

void Extractor::pushToTransporter(CommandMessage *msg)
{
    if (m_direction != CommandToTransporter)
        return;
    return m_transporter->putMessage(msg);
}

void Extractor::appendCommand(Command *command)
{
    m_commands.append(command);
}
