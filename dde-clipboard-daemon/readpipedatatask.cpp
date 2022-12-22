// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifdef USE_DEEPIN_KF5_WAYLAND
#include "readpipedatatask.h"

#include <QDebug>
#include <QFile>
#include <utility>

#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/datacontroloffer.h>

#include <unistd.h>

ReadPipeDataTask::ReadPipeDataTask(ConnectionThread *connectionThread, DataControlOfferV1 *offerV1, QString mimeType, QObject *parent)
        : QObject(parent)
        , m_stopRunning(false)
        , m_mimeType(mimeType)
        , m_pOffer(offerV1)
        , m_pConnectionThread(connectionThread)
{

}

void ReadPipeDataTask::stopRunning()
{
    QMutexLocker locker(&m_mutexLock);
    m_stopRunning = true;
}

void ReadPipeDataTask::run()
{
    if (!m_pConnectionThread || !m_pOffer || m_mimeType.isEmpty())
        return;

    int pipeFds[2];
    if (pipe(pipeFds) != 0) {
        qWarning() << "Create pipe failed.";

        // 避免返回数据量少
        Q_EMIT dataReady((qint64)m_pOffer, m_mimeType, QByteArray());
        return;
    }

    // 根据mime类取数据，写入pipe中
    m_pOffer->receive(m_mimeType, pipeFds[1]);
    close(pipeFds[1]);

    QByteArray data;
    readData(pipeFds[0], data);

    Q_EMIT dataReady((qint64)m_pOffer, m_mimeType, data);

    close(pipeFds[0]);
    m_stopRunning = false;
}

bool ReadPipeDataTask::readData(int fd, QByteArray &data)
{
    QFile readPipe;
    if (!readPipe.open(fd, QIODevice::ReadOnly))
        return false;

    if (!readPipe.isReadable()) {
        qWarning() << "Pipe is not readable";
        readPipe.close();
        return false;
    }

    int retCount = 0;
    do {
        m_mutexLock.lock();
        bool needStopRunning = m_stopRunning;
        m_mutexLock.unlock();

        if (needStopRunning) {
            data.clear();
            readPipe.close();
            return false;
        }

        QByteArray bytes = readPipe.read(1024 * 4);
        retCount = bytes.count();
        if (!bytes.isEmpty())
            data.append(bytes);
    }while(retCount);

    readPipe.close();

    return true;
}

#endif // USE_DEEPIN_KF5_WAYLAND

