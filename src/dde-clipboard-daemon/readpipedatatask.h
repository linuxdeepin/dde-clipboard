// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifdef USE_DEEPIN_KF5_WAYLAND
#ifndef READ_PIPE_DATA_TASK_H
#define READ_PIPE_DATA_TASK_H

#include <QObject>
#include <QRunnable>
#include <QMutex>

namespace KWayland
{
    namespace Client
    {
        class ConnectionThread;
        class DataControlOfferV1;
    } //Client
} //KWayland

using namespace KWayland::Client;

class ReadPipeDataTask : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit ReadPipeDataTask(ConnectionThread *connectionThread, DataControlOfferV1 *offerV1,
                              QString mimeType, QObject *parent = nullptr);

    void stopRunning();

signals:
    void dataReady(qint64, const QString &, const QByteArray &);

protected:
    void run() override;

private:
    bool readData(int fd, QByteArray &data);

private:
    bool m_stopRunning;
    QString m_mimeType;
    QMutex m_mutexLock;

    DataControlOfferV1 *m_pOffer;
    ConnectionThread *m_pConnectionThread;
};


#endif //READ_PIPE_DATA_TASK_H
#endif // USE_DEEPIN_KF5_WAYLAND