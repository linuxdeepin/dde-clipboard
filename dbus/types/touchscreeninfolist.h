// Copyright (C) 2011 ~ 2017 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TOUCHSCREENINFOLIST_H
#define TOUCHSCREENINFOLIST_H

#include <QString>
#include <QList>
#include <QDBusMetaType>

struct TouchscreenInfo {
    qint32 id;
    QString name;
    QString deviceNode;
    QString serialNumber;

    bool operator ==(const TouchscreenInfo& info);
    bool operator !=(const TouchscreenInfo& info);
};

inline bool operator==(const TouchscreenInfo& lhs, const TouchscreenInfo& rhs)
    { return lhs.id == rhs.id && lhs.name == rhs.name && lhs.deviceNode == rhs.deviceNode && lhs.serialNumber == rhs.serialNumber; }
inline bool operator!=(const TouchscreenInfo& lhs, const TouchscreenInfo& rhs)
    { return !(lhs == rhs); }

typedef QList<TouchscreenInfo> TouchscreenInfoList;

Q_DECLARE_METATYPE(TouchscreenInfo)
Q_DECLARE_METATYPE(TouchscreenInfoList)

QDBusArgument &operator<<(QDBusArgument &arg, const TouchscreenInfo &info);
const QDBusArgument &operator>>(const QDBusArgument &arg, TouchscreenInfo &info);

void registerTouchscreenInfoListMetaType();

#endif // !TOUCHSCREENINFOLIST_H
