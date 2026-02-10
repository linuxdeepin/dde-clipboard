// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "messagemanager.h"
#include "constants.h"

#include <DFloatingMessage>
#include <QTimer>

MessageManager::MessageManager(QObject *parent)
    : QObject(parent)
    , m_autoHideTimer(new QTimer(this))
{
    m_autoHideTimer->setSingleShot(true);
    m_autoHideTimer->setInterval(MessageAutoHideDelay);
    connect(m_autoHideTimer, &QTimer::timeout, this, &MessageManager::hideCurrentMessage);
}

MessageManager *MessageManager::instance()
{
    static MessageManager instance;
    return &instance;
}

void MessageManager::sendMessage(QWidget *par)
{
    if (m_activeParent) {
        hideCurrentMessage();
    }

    Dtk::Widget::DFloatingMessage *floMsg = new Dtk::Widget::DFloatingMessage(Dtk::Widget::DFloatingMessage::TransientType, par);
    floMsg->setAttribute(Qt::WA_DeleteOnClose);
    floMsg->setAttribute(Qt::WA_AlwaysStackOnTop);
    // 固定使用当前图标，不跟随系统图标变化
    floMsg->setIcon(QIcon(":/dsg/built-in-icons/dialog-ok.dci"));
    floMsg->setMessage(tr("Copied"));
    floMsg->setMaximumWidth(par->rect().marginsRemoved(QMargins(10, 0, 10, 0)).width());

    QRect geometry(QPoint(0, 0), floMsg->sizeHint());
    geometry.moveCenter(par->rect().center());
    geometry.moveBottom(par->rect().bottom());
    floMsg->setGeometry(geometry);
    floMsg->show();

    m_activeParent = par;
    m_autoHideTimer->start();
}

void MessageManager::hideCurrentMessage()
{
    if (!m_activeParent) {
        return;
    }

    m_autoHideTimer->stop();

    QWidget *par = m_activeParent.data();
    QList<Dtk::Widget::DFloatingMessage *> messages = par->findChildren<Dtk::Widget::DFloatingMessage *>(QString(), Qt::FindDirectChildrenOnly);
    for (Dtk::Widget::DFloatingMessage *msg : messages) {
        if (msg->messageType() == Dtk::Widget::DFloatingMessage::TransientType) {
            msg->hide();
            msg->deleteLater();
        }
    }

    m_activeParent.clear();
}
