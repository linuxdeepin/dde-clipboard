// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "qwayland-wlr-data-control-unstable-v1.h"
#include <QObject>

class WlrDataControlSourceIntegration :
    public QObject,
    public QtWayland::zwlr_data_control_source_v1
{
    Q_OBJECT
public:
    WlrDataControlSourceIntegration(::zwlr_data_control_source_v1 *object);
    virtual ~WlrDataControlSourceIntegration() override;

protected:
    virtual void zwlr_data_control_source_v1_send(const QString &mime_type, int32_t fd) override;
    virtual void zwlr_data_control_source_v1_cancelled() override;

Q_SIGNALS:
    void send(QString mimeType, int fd);
    void cancelled();
};
