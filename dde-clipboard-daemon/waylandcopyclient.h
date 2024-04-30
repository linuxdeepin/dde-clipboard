// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "qwayland-wlr-data-control-unstable-v1.h"

#include <wayland-client-core.h>

#include <QByteArray>
#include <QMimeData>
#include <QObject>
#include <QScopedPointer>
#include <QtWaylandClient/QWaylandClientExtensionTemplate>
#include <QtWaylandClient/private/qwaylandintegration_p.h>

class ZDataControlDeviceV1;
class ZDataControlOfferV1;

class ZWaylandDataControlManager
    : public QWaylandClientExtensionTemplate<ZWaylandDataControlManager>,
      public QtWayland::zwlr_data_control_manager_v1
{
    Q_OBJECT
public:
    explicit ZWaylandDataControlManager(QObject *parent = nullptr);

    void initBase();
    void setMimeData(QMimeData *mimeData);

signals:
    void clipboardChanged(const QMimeData *data);

private:
    ZDataControlDeviceV1 *m_device;
};

class ZDataControlDeviceV1 : public QObject, public QtWayland::zwlr_data_control_device_v1
{
    Q_OBJECT

public:
    explicit ZDataControlDeviceV1(::zwlr_data_control_device_v1 *device, QObject *parent = nullptr);

signals:
    void clipboardChanged(const QMimeData *data);

protected:
    void zwlr_data_control_device_v1_selection(struct ::zwlr_data_control_offer_v1 *id) override;
    void
    zwlr_data_control_device_v1_primary_selection(struct ::zwlr_data_control_offer_v1 *id) override;
    void zwlr_data_control_device_v1_data_offer(struct ::zwlr_data_control_offer_v1 *id) override;

    bool readData(int fd, QByteArray &data);

private:
    QScopedPointer<ZDataControlOfferV1> m_dataoffer;
    QMimeData *m_data;
};

class ZDataControlOfferV1 : public QObject, public QtWayland::zwlr_data_control_offer_v1
{
    Q_OBJECT
    friend ZDataControlDeviceV1;

public:
    explicit ZDataControlOfferV1(::zwlr_data_control_offer_v1 *device, QObject *parent = nullptr);

protected:
    void zwlr_data_control_offer_v1_offer(const QString &mime_type) override;

private:
    QStringList m_mimetypes;
};

class ZDataControlResourceV1 : public QObject, public QtWayland::zwlr_data_control_source_v1
{
    Q_OBJECT
public:
    ZDataControlResourceV1(::zwlr_data_control_source_v1 *resource,
                           QMimeData *data,
                           QObject *parent = nullptr);

protected:
    void zwlr_data_control_source_v1_send(const QString &mime_type, int32_t fd) override;

private:
    QMimeData *m_mimeData;
};
