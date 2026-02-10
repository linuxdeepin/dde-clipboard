// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MESSAGEMANAGER_H
#define MESSAGEMANAGER_H

#include <QObject>
#include <QWidget>
#include <QPointer>
#include <QTimer>

class MessageManager : public QObject
{
    Q_OBJECT

public:
    static MessageManager *instance();

    /*!
     * @brief 发送浮动消息
     * @param par 父窗口
     */
    void sendMessage(QWidget *par);

    /*!
     * @brief 隐藏当前消息
     */
    void hideCurrentMessage();

private:
    explicit MessageManager(QObject *parent = nullptr);
    ~MessageManager() override = default;
    

    Q_DISABLE_COPY(MessageManager)
    
    QPointer<QWidget> m_activeParent;
    QTimer *m_autoHideTimer;
};

#endif // MESSAGEMANAGER_H
