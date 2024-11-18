// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QObject>
#include <optional>
#include "qwayland-wlr-data-control-unstable-v1.h"

class WlrDataControlDeviceIntegration;

class WlrDataControlOfferIntegration :
    public QObject,
    public QtWayland::zwlr_data_control_offer_v1
{
    Q_OBJECT
public:
    WlrDataControlOfferIntegration(::zwlr_data_control_offer_v1 *object);
    virtual ~WlrDataControlOfferIntegration() override;

    QStringList availableMimeTypes() { return m_mimeTypes; }
    void setAvailableMimeTypes(QStringList mimeTypes) { m_mimeTypes = mimeTypes; }

protected:
    virtual void zwlr_data_control_offer_v1_offer(const QString &mime_type) override {
        Q_EMIT advertiseMimeType(mime_type);
    }

private:
    QStringList m_mimeTypes;

Q_SIGNALS:
    void advertiseMimeType(QString mimeType);
};
