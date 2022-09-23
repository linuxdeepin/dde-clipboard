// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "command.h"
#include "servicetool.h"

Command::Command(CommandService *service)
    : m_service(service)
{
    setAutoDelete(true);
}

void Command::exit()
{
    m_isExit = true;
}

Command::State Command::state() const
{
    return m_state;
}

CommandService *Command::service() const
{
    return m_service;
}

void Command::installMessageExtractor(Extractor *extractor)
{
    Q_ASSERT(extractor);
    switch (extractor->direction()) {
    case Extractor::CommandToTransporter: {
        if (m_outEx) {
            qWarning("This command already has the out direction Extractor.");
            return;
        }
        m_outEx = extractor;
    }
        break;
    case Extractor::TransporterToCommand: {
        if (m_inEx) {
            qWarning("This command already has the in direction Extractor.");
            return;
        }
        m_inEx = extractor;
    }
        break;
    }
}

void Command::run()
{
    Q_ASSERT(m_inEx);
    while (true) {
        m_state = Preparing;
        CommandMessage *msg = m_inEx->takeFromTransporter();
        if (m_isExit) {
            delete msg;
            break;
        }

        m_state = Running;
        QList<CommandMessage *> destMsgs = doExecute(msg);

        // 允许复用同一个 msg 的情况
        if (!destMsgs.contains(msg))
            delete msg;

        if (m_outEx) {
            for (auto destMsg : destMsgs)
                m_outEx->pushToTransporter(destMsg);
        }
    }
    m_state = Preparing;
}
