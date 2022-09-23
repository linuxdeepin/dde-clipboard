// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <qobject.h>
#include <qobjectdefs.h>
#include <qrunnable.h>
#ifdef USE_DEEPIN_KF5_WAYLAND
#ifndef COPYCLIENT_H
#define COPYCLIENT_H

#include <QMutex>
#include <QMimeData>
#include <QPointer>
#include <QMimeType>
#include <QWaitCondition>

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

class CommandServiceManager;
class CommandService;
class WaylandCopyClient : public QObject
{
    Q_OBJECT
    friend class RequestReceiveCommand;

public:
    explicit WaylandCopyClient(QObject *parent = nullptr);
    virtual ~WaylandCopyClient();

    void init();
    const QMimeData *mimeData();
    void setMimeData(QMimeData *mimeData);
    void sendOffer();
    void call();

    void setupRegistry(Registry *registry);
private:
    void setupRegistry();
    QStringList filterMimeType(const QStringList &mimeTypeList);
    void cleanServiceFlow(CommandService *service);
    void wakePipeSyncCondition();

Q_SIGNALS:
    void dataChanged();

protected slots:
    void onThreadConnected();
    void onSendDataRequest(const QString &mimeType, qint32 fd);
    void onDataOffered(DataControlOfferV1 *offer);
    void onRegistrySeatAnnounced(quint32 name, quint32 version);
    void onDeviceManagerAnnounced(quint32 name, quint32 version);
    void onServiceFlowFinished(bool success);

private:
    QThread *m_connectionThread;
    ConnectionThread *m_connectionThreadObject;
    EventQueue *m_eventQueue;
    DataControlDeviceManager *m_dataControlDeviceManager;
    DataControlDeviceV1 *m_dataControlDevice;
    DataControlSourceV1 *m_copyControlSource;
    QPointer<QMimeData> m_mimeData;
    Seat *m_seat;
    CommandServiceManager *m_manager;
    Registry *m_registry;
    QList<CommandService *> m_runningRootService;
    QMutex m_pipeSyncMutex;
    // REMOVE IT: 窗管无法支持异步执行,因此使用服务流时需要额外的进行同步操作
    // 否则可能出现访问野指针的风险.
    QWaitCondition m_pipeSyncCondition;
    QList<CommandService *> m_writeServices;
};

#endif // COPYCLIENT_H
#endif
