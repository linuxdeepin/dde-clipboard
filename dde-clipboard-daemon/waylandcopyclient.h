// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifdef USE_DEEPIN_KF5_WAYLAND
#ifndef COPYCLIENT_H
#define COPYCLIENT_H

#include <QMutex>
#include <QMimeData>
#include <QPointer>
#include <QMimeType>

namespace KWayland
{
namespace Client
{
class ConnectionThread;
class EventQueue;
class Registry;
class Seat;
class DataControlDeviceV1;
class DataControlDeviceManager;
class DataControlSourceV1;
class DataControlOfferV1;
} //Client
} //KWayland

class ReadPipeDataTask;

using namespace KWayland::Client;

class DMimeData : public QMimeData
{
    Q_OBJECT
public:
    DMimeData();
    ~DMimeData();
    virtual QVariant retrieveData(const QString &mimeType,
                                      QVariant::Type preferredType) const;
};

class WaylandCopyClient : public QObject
{
    Q_OBJECT

public:
    explicit WaylandCopyClient(QObject *parent = nullptr);
    virtual ~WaylandCopyClient();

    void init();
    const QMimeData *mimeData();
    void setMimeData(QMimeData *mimeData);
    void sendOffer();

private:
    void setupRegistry(Registry *registry);
    QStringList filterMimeType(const QStringList &mimeTypeList);

Q_SIGNALS:
    void dataChanged();

protected slots:
    void onSendDataRequest(const QString &mimeType, qint32 fd) const;
    void onDataOffered(DataControlOfferV1 *offer);
    void onDataChanged();

private:
    void execTask(const QStringList &mimeTypes, DataControlOfferV1 *offer);
    void tryStopOldTask();
    void taskDataReady(qint64, const QString &mimeType, const QByteArray &data);

private:
    QThread *m_connectionThread;
    ConnectionThread *m_connectionThreadObject;
    EventQueue *m_eventQueue;
    DataControlDeviceManager *m_dataControlDeviceManager;
    DataControlDeviceV1 *m_dataControlDevice;
    DataControlSourceV1 *m_copyControlSource;
    QPointer<QMimeData> m_mimeData;
    Seat *m_seat;

    qint64 m_curOffer;
    QStringList m_curMimeTypes;
    QList<ReadPipeDataTask *> m_tasks;
};

#endif // COPYCLIENT_H
#endif
