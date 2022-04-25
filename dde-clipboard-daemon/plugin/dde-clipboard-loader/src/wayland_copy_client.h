/*
 * Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     yinjie <yinjie@uniontech.com>
 *
 * Maintainer: yinjie <yinjie@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef WAYLANDCOPYCLIENT_H
#define WAYLANDCOPYCLIENT_H


#include "iteminfo.h"

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
class DataDeviceManager;
class DataDevice;
class DataSource;
class DataOffer;
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


class WaylandCopyClient : public QObject
{
    Q_OBJECT

public:
    virtual ~WaylandCopyClient();

    void init();
    const QMimeData *mimeData();
    void setMimeData(QMimeData *mimeData);
    void sendOffer();
    static WaylandCopyClient& ref();

private:
    explicit WaylandCopyClient(QObject *parent = nullptr);

    void setupRegistry(Registry *registry);
    QList<QMimeType> filterMimeType(const QList<QMimeType> &mimeTypeList);

Q_SIGNALS:
    void dataChanged();

protected slots:
    void onSendDataRequest(const QString &mimeType, qint32 fd) const;
    void onDataOffered(DataOffer *offer);

private:
    QThread *m_connectionThread;
    ConnectionThread *m_connectionThreadObject;
    EventQueue *m_eventQueue;
    DataDeviceManager *m_dataControlDeviceManager;
    DataDevice *m_dataControlDevice;
    DataSource *m_copyControlSource;
    QPointer<QMimeData> m_mimeData;
    Seat *m_seat;
    ItemInfo m_itemInfo;
};

#endif // WAYLANDCOPYCLIENT_H
