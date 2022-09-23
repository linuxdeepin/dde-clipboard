// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "waylandcopyservice.h"
#include "../waylandcopyclient.h"

#include <KWayland/Client/datacontroloffer.h>
#include <unistd.h>
#include <QDebug>
#include <QFile>
#include <QImageWriter>
#include <QImage>
#include <QBuffer>
#include <QUrl>

QList<CommandMessage *> MimeDataFilterCommand::doExecute(CommandMessage *msg)
{
    auto pipeMsg = static_cast<PipeCommandMessage *>(msg);
    const QStringList &types = filterMimeType(pipeMsg->srcMimeTypes);

    if (types.isEmpty()) {
        CommandMessage *msg = new CommandMessage();
        msg->setError(CommandMessage::ExecuteError);
        return {msg};
    }

    pipeMsg->dstMimeTypes = types;
    return {pipeMsg};
}

QString MimeDataFilterCommand::name() const
{
    return QLatin1String("MimeDataFilterCommand");
}

QStringList MimeDataFilterCommand::filterMimeType(const QStringList &mimeTypeList)
{
    QStringList tmpList;
    for (const QString &mimeType : mimeTypeList) {
        // 根据窗管的要求，不读取纯大写、和不含'/'的字段，因为源窗口可能没有写入这些字段的数据，导致获取数据的线程一直等待。
        if ((mimeType.contains("/") && mimeType.toUpper() != mimeType)
                || mimeType == "FROM_DEEPIN_CLIPBOARD_MANAGER"
                || mimeType == "TIMESTAMP") {
            tmpList.append(mimeType);
        }
    }

    return tmpList;
}

QList<CommandMessage *> RequestReceiveCommand::doExecute(CommandMessage *msg)
{
    if (msg->error() != CommandMessage::NoError)
        return {msg};

    auto pipeMsg = static_cast<PipeCommandMessage *>(msg);
    QList<CommandMessage *> destCommandMsgs;

    for (auto mimeType : pipeMsg->dstMimeTypes) {
        int pipeFds[2];
        if (pipe(pipeFds) != 0) {
            qWarning() << "Create pipe failed.";
            continue;
        }

        // 根据mime类取数据，写入pipe中
        if (!pipeMsg->offer || !pipeMsg->offer->isValid()) {
            close(pipeFds[0]);
            close(pipeFds[1]);
            pipeMsg->setError(CommandMessage::ExecuteError);
            return {msg};
        }

        // ### lock or not ?
        pipeMsg->offer->receive(mimeType, pipeFds[1]);
        close(pipeFds[1]);

        CopyCommandMessage *copyMsg = new CopyCommandMessage();
        copyMsg->pipeFds[0] = pipeFds[0];
        copyMsg->pipeFds[1] = pipeFds[1];
        copyMsg->mimeData = pipeMsg->mimeData;
        copyMsg->srcMimeTypes = pipeMsg->srcMimeTypes;
        copyMsg->dstMimeTypes = pipeMsg->dstMimeTypes;
        copyMsg->mimeTypeCount = pipeMsg->dstMimeTypes.count();
        copyMsg->currentMimeType = mimeType;

        destCommandMsgs.append(copyMsg);
    }

    if (auto recvService = dynamic_cast<RequestReceiveService *>(service()))
        static_cast<WaylandCopyClient *>(recvService->parent())->wakePipeSyncCondition();

    return destCommandMsgs;
}

QString RequestReceiveCommand::name() const
{
    return  QLatin1String("RequestReceiveCommand");
}

QList<CommandMessage *> ReadDataCommand::doExecute(CommandMessage *msg)
{
    if (msg->error() != CommandMessage::NoError)
        return {msg};

    auto copyMsg = static_cast<CopyCommandMessage *>(msg);
    QFile readPipeDevice;
    QByteArray data;

    do {
        if (!readPipeDevice.open(copyMsg->pipeFds[0], QIODevice::ReadOnly)) {
            qInfo() << "Open pipe failed!";
            break;
        }

        if (!readPipeDevice.isReadable()) {
            qInfo() << "Pipe is not readable";
            break;
        }

        data = readPipeDevice.readAll();

        if (data.isEmpty()) {
            qWarning() << "Pipe data is empty, mime type: " << copyMsg->currentMimeType;
        }
    } while (false);

    close(copyMsg->pipeFds[0]);
    copyMsg->data = data;
    return {copyMsg};
}

QString ReadDataCommand::name() const
{
    return QLatin1String("ReadDataCommand");
}

QList<Command *> SyncMimeDataService::commands()
{
    return {new SyncMimeDataCommand(this)};
}

QList<CommandMessage *> SyncMimeDataCommand::doExecute(CommandMessage *msg)
{
    bool finished = false;
    bool hasError = false;

    do {
        if (msg->error() != CommandMessage::NoError) {
            hasError = true;
            break;
        }

        auto copyMsg = static_cast<CopyCommandMessage *>(msg);
        QMutexLocker locker(&m_mutex);
        if (m_mimeCount.load() == 0) {
            copyMsg->mimeData->clear();
        }

        copyMsg->mimeData->setData(copyMsg->currentMimeType, copyMsg->data);
        m_mimeCount++;

        if (m_mimeCount.load() == copyMsg->mimeTypeCount) {
            // service Finished.
            finished = true;
        }
    } while (false);

    auto syncService = dynamic_cast<SyncMimeDataService *>(service());
    if (finished)
        Q_EMIT syncService->finished(!hasError);

    return {};
}

QString SyncMimeDataCommand::name() const
{
    return QLatin1String("SyncMimeDataCommand");
}

QList<Command *> MimeDataFilterService::commands()
{
    return {new MimeDataFilterCommand(this)};
}

QList<Command *> RequestReceiveService::commands()
{
    return {new RequestReceiveCommand(this)};
}

QList<Command *> ReadDataService::commands()
{
    QList<Command *> commands;
    for (int i = 0; i < MaxReadCommandCount; ++i)
        commands.append(new ReadDataCommand(this));

    return commands;
}

static QByteArray getByteArray(QMimeData *mimeData, const QString &mimeType)
{
    QByteArray content;
    if (mimeType == QLatin1String("text/plain")) {
        content = mimeData->text().toUtf8();
    } else if (mimeData->hasImage()
               && (mimeType == QLatin1String("application/x-qt-image")
                   || mimeType.startsWith(QLatin1String("image/")))) {
        QImage image = qvariant_cast<QImage>(mimeData->imageData());
        if (!image.isNull()) {
            QBuffer buf;
            buf.open(QIODevice::ReadWrite);
            QByteArray fmt = "BMP";
            if (mimeType.startsWith(QLatin1String("image/"))) {
                QByteArray imgFmt = mimeType.mid(6).toUpper().toLatin1();
                if (QImageWriter::supportedImageFormats().contains(imgFmt))
                    fmt = imgFmt;
            }
            QImageWriter wr(&buf, fmt);
            wr.write(image);
            content = buf.buffer();
        }
    } else if (mimeType == QLatin1String("application/x-color")) {
        content = qvariant_cast<QColor>(mimeData->colorData()).name().toLatin1();
    } else if (mimeType == QLatin1String("text/uri-list")) {
        QList<QUrl> urls = mimeData->urls();
        for (int i = 0; i < urls.count(); ++i) {
            content.append(urls.at(i).toEncoded());
            content.append('\n');
        }
    } else {
        content = mimeData->data(mimeType);
    }
    return content;
}

QList<CommandMessage *> CollectWrittenDataCommand::doExecute(CommandMessage *msg)
{
    WriteDataMessage *writeMsg = static_cast<WriteDataMessage *>(msg);
    const QByteArray &ba = getByteArray(writeMsg->mimeData, writeMsg->mimeType);
    writeMsg->data = ba;

    return {writeMsg};
}

QString CollectWrittenDataCommand::name() const
{
    return QLatin1String("CollectWrittenDataCommand");
}

QList<CommandMessage *> WriteDataToFDCommand::doExecute(CommandMessage *msg)
{
    WriteDataMessage *writeMsg = static_cast<WriteDataMessage *>(msg);

    QFile f;
    if (f.open(writeMsg->fd, QFile::WriteOnly, QFile::AutoCloseHandle)) {
        f.write(writeMsg->data);
        f.close();
    }

    return {};
}

QString WriteDataToFDCommand::name() const
{
    return QLatin1String("WriteDataToFDCommand");
}

#define WRITEDATASERVICECOUNT 4

QList<Command *> CollectWrittenDataService::commands()
{
    QList<Command *> commands;
    for (int i = 0; i < WRITEDATASERVICECOUNT; ++i)
        commands.append(new CollectWrittenDataCommand());

    return commands;
}

QList<Command *> WriteDataToFDService::commands()
{
    QList<Command *> commands;
    for (int i = 0; i < WRITEDATASERVICECOUNT; ++i)
        commands.append(new WriteDataToFDCommand());

    return commands;
}
