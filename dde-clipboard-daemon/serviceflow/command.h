// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMMAND
#define COMMAND

#include <QRunnable>

class CommandService;
class CommandMessage;
class Extractor;
class Command : public QRunnable
{
public:
    enum State {
        Preparing,
        Running
    };
    Command(CommandService *service = nullptr);

    virtual QList<CommandMessage *> doExecute(CommandMessage *) = 0;
    virtual QString name() const = 0;

    void exit();
    State state() const;

    CommandService *service() const;
    void installMessageExtractor(Extractor *extractor);

private:
    void run() override;

    Extractor * m_inEx = nullptr;
    Extractor * m_outEx = nullptr;
    bool m_isExit = false;
    State m_state = Preparing;
    CommandService *m_service = nullptr;
};

#endif  // COMMANDMESSAGE
