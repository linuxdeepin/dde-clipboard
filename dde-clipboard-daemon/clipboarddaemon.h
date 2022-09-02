// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CLIPBOARD_DAEMON_H
#define CLIPBOARD_DAEMON_H

#include <QObject>

class ClipboardDaemon : public QObject
{
    Q_OBJECT
public:
    explicit ClipboardDaemon(QObject *parent = nullptr);
};

#endif //CLIPBOARD_DAEMON_H
