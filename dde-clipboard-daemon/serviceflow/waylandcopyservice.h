// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WAYLANDCOPYSERVICE_H
#define WAYLANDCOPYSERVICE_H

#include "commandmessage.h"
#include "command.h"
#include "commandservice.h"

#include <QMimeData>
#include <QMutex>
#include <QSharedPointer>

namespace KWayland
{
namespace Client
{
class DataControlOfferV1;
} //Client
} //KWayland


// for read data.
class PipeCommandMessage : public CommandMessage
{
public:
    KWayland::Client::DataControlOfferV1 *offer;
    int pipeFds[2] = {0};
    QMimeData *mimeData;
    QStringList srcMimeTypes;
    QStringList dstMimeTypes;
};

class CopyCommandMessage : public PipeCommandMessage
{
public:
    int mimeTypeCount;
    QString currentMimeType;
    QByteArray data;
};

class MimeDataFilterCommand : public Command
{
public:
    using Command::Command;
    QList<CommandMessage *> doExecute(CommandMessage *msg) override;
    QString name() const override;

    QStringList filterMimeType(const QStringList &mimeTypeList);
};

class RequestReceiveCommand : public Command
{
public:
    using Command::Command;
    QList<CommandMessage *> doExecute(CommandMessage *msg) override;
    QString name() const override;
};

class ReadDataCommand : public Command
{
public:
    using Command::Command;
    QList<CommandMessage *> doExecute(CommandMessage *msg) override;
    QString name() const override;
};

class SyncMimeDataCommand : public Command
{
public:
    using Command::Command;
    QList<CommandMessage *> doExecute(CommandMessage *msg) override;
    QString name() const override;

private:
    QAtomicInt m_mimeCount = 0;
    QMutex m_mutex;
};

class MimeDataFilterService : public CommandService
{
public:
    QList<Command *> commands() override;
};

class RequestReceiveService : public QObject, public CommandService
{
    Q_OBJECT

public:
    using QObject::QObject;
    QList<Command *> commands() override;
};

class ReadDataService : public CommandService
{
public:
    enum { MaxReadCommandCount = 8 };
    QList<Command *> commands() override;
};

class SyncMimeDataService : public QObject, public CommandService
{
    Q_OBJECT
public:
    QList<Command *> commands() override;

Q_SIGNALS:
    void finished(bool success);
};

// for write data
class WriteDataMessage : public CommandMessage
{
public:
    QString mimeType;
    QByteArray data;
    QMimeData *mimeData;
    qint32 fd;
};

class CollectWrittenDataCommand : public Command
{
public:
    QList<CommandMessage *> doExecute(CommandMessage *);
    QString name() const;
};

class WriteDataToFDCommand : public Command
{
public:
    QList<CommandMessage *> doExecute(CommandMessage *);
    QString name() const;
};

class CollectWrittenDataService : public CommandService
{
public:
    QList<Command *> commands();
};

class WriteDataToFDService : public CommandService
{
public:
    QList<Command *> commands();
};
#endif // WAYLANDCOPYSERVICE_H
