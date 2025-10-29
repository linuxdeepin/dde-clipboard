// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MESSAGEWINDOW_H
#define MESSAGEWINDOW_H

#include <DBlurEffectWidget>
#include <DWindowManagerHelper>
#include <DPlatformWindowHandle>
#include <DGuiApplicationHelper>
#include <DIconButton>

#include <QWidget>
#include <QLabel>
#include <QTimer>
#include <QIcon>
#include <QEvent>
#include <QPointer>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

// 前置声明
class MessageWindow;

/*!
 * \~chinese \struct MessageWindowInfo
 * \~chinese \brief 消息窗口信息结构
 */
struct MessageWindowInfo {
    QRect rect;                     // 窗口显示区域
    QPointer<MessageWindow> messageWindow;  // 窗口指针
};

// 判断是否支持 DBlurEffectWithBorderWidget
#ifdef DTKWIDGET_CLASS_DBlurEffectWithBorderWidget
class MessageWindow : public DBlurEffectWithBorderWidget
#else
class MessageWindow : public DBlurEffectWidget
#endif
{
    Q_OBJECT

public:
    explicit MessageWindow(QWidget *parent = nullptr);
    ~MessageWindow();

    /*!
     * \~chinese \brief 设置图标
     * \~chinese \param icon 要显示的图标
     */
    void setIcon(const QIcon &icon);
    
    /*!
     * \~chinese \brief 设置消息文本
     * \~chinese \param msg 消息内容
     */
    void setMessage(const QString &msg);
    
    /*!
     * \~chinese \brief 设置显示持续时间
     * \~chinese \param msec 持续时间（毫秒），0表示不自动关闭
     */
    void setDuration(int msec);

protected:
    void showEvent(QShowEvent *event) override;

private:
    DWindowManagerHelper *m_wmHelper;
    DGuiApplicationHelper::ColorType m_themeType;
    DPlatformWindowHandle m_platformWindowHandle;
    DIconButton *m_iconButton;
    QLabel *m_message;
    QTimer *m_timer;
    int m_duration;
};

/*!
 * \~chinese \class MessageWindowManager
 * \~chinese \brief 消息窗口管理器，管理多个提示窗口的显示
 */
class MessageWindowManager : public QObject
{
    Q_OBJECT

public:
    explicit MessageWindowManager(QObject *parent = nullptr);
    ~MessageWindowManager();

    /*!
     * \~chinese \brief 获取单例实例
     */
    static MessageWindowManager *instance();

    /*!
     * \~chinese \brief 更新显示区域
     * \~chinese \param rect 新的显示区域
     */
    void updateDisplayRect(const QRect &rect);
    
    /*!
     * \~chinese \brief 显示消息提示
     * \~chinese \param msg 消息内容
     * \~chinese \param icon 图标
     * \~chinese \param duration 显示持续时间（毫秒）
     */
    void showMessage(const QString &msg, const QIcon &icon = QIcon(), int duration = 3000);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    MessageWindowInfo m_messageWindowInfo[3]; // 最多支持3个提示窗口
};

#endif // MESSAGEWINDOW_H

