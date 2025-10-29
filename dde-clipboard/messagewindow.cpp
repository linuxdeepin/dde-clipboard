// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "messagewindow.h"
#include "constants.h"

#include <DGuiApplicationHelper>

#include <QHBoxLayout>
#include <QScreen>

Q_GLOBAL_STATIC(MessageWindowManager, messageWindowManager)

MessageWindow::MessageWindow(QWidget *parent)
#ifdef DTKWIDGET_CLASS_DBlurEffectWithBorderWidget
    : DBlurEffectWithBorderWidget(parent)
#else
    : DBlurEffectWidget(parent)
#endif
    , m_wmHelper(DWindowManagerHelper::instance())
    , m_themeType(DGuiApplicationHelper::instance()->themeType())
    , m_platformWindowHandle(this)
    , m_timer(new QTimer(this))
    , m_duration(0)
{
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);
    setWindowFlag(Qt::WindowDoesNotAcceptFocus);

    setFixedHeight(TipsWindowHeight);
    m_platformWindowHandle.setWindowRadius(15);
    // 禁止窗口可以被鼠标拖动
    m_platformWindowHandle.setEnableSystemMove(false);

    m_iconButton = new DIconButton(this);
    m_iconButton->setFlat(true);
    m_iconButton->setFocusPolicy(Qt::NoFocus);
    m_iconButton->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_iconButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_iconButton->setIconSize(QSize(28, 28));

    m_message = new QLabel(this);
    m_message->setForegroundRole(QPalette::WindowText);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 0, 16, 0);
    layout->addWidget(m_iconButton);
    layout->addWidget(m_message);

    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, [ this ] {
        this->close();
    });

    auto updateTheme = [ = ] (DGuiApplicationHelper::ColorType themeType) {
        if (themeType == DGuiApplicationHelper::LightType) {
            setMaskColor(QColor(247, 247, 247));
            setMaskAlpha(quint8(255 * 0.6));
        } else {
            setMaskColor(QColor(42, 42, 42));
            setMaskAlpha(quint8(255 * 0.8));
        }
    };

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, updateTheme);
    updateTheme(m_themeType);
}

MessageWindow::~MessageWindow()
{

}

void MessageWindow::setIcon(const QIcon &icon)
{
    m_iconButton->setIcon(icon);
}

void MessageWindow::setMessage(const QString &msg)
{
    m_message->setText(msg);
}

void MessageWindow::setDuration(int msec)
{
    m_duration = msec;
}

void MessageWindow::showEvent(QShowEvent *event)
{
    if (m_duration > 0) {
        m_timer->start(m_duration);
    }

#ifdef DTKWIDGET_CLASS_DBlurEffectWithBorderWidget
    return DBlurEffectWithBorderWidget::showEvent(event);
#else
    return DBlurEffectWidget::showEvent(event);
#endif
}

MessageWindowManager::MessageWindowManager(QObject *parent)
    : QObject(parent)
{
    for (int i = 0; i < MaxTipsWindowNum; i++) {
        m_messageWindowInfo[i].rect = QRect();
        m_messageWindowInfo[i].messageWindow = nullptr;
    }
}

MessageWindowManager::~MessageWindowManager()
{

}

MessageWindowManager *MessageWindowManager::instance()
{
    return messageWindowManager;
}

void MessageWindowManager::updateDisplayRect(const QRect &rect)
{
    // 更新窗口显示的位置
    for (int i = 0; i < MaxTipsWindowNum; i++) {
        QRect rt = rect;
        rt.setHeight(TipsWindowHeight);
        // 计算Y坐标：屏幕底部向上偏移，每个窗口堆叠显示
        int yPos = rect.y() + rect.height() - (TipsWindowHeight + WindowMargin) * (i + 1);
        rt.moveTo(rect.x(), yPos);
        m_messageWindowInfo[i].rect = rt;

        // 如果当前区域已经有弹窗，需要同步更新其位置
        if (m_messageWindowInfo[i].messageWindow) {
            // 水平居中，垂直使用计算好的位置
            int xPos = rt.x() + (rt.width() - m_messageWindowInfo[i].messageWindow->width()) / 2;
            m_messageWindowInfo[i].messageWindow->move(xPos, yPos);
        }
    }
}

void MessageWindowManager::showMessage(const QString &msg, const QIcon &icon, int duration)
{
    // 查找可以弹窗的位置，最多只能有三个弹窗，如果大于三个则不处理
    for (int i = 0; i < MaxTipsWindowNum; i++) {
        if (!m_messageWindowInfo[i].messageWindow) {
            m_messageWindowInfo[i].messageWindow = new MessageWindow;
            // 窗口关闭时，会自动delete；由于messageWindow为QPointer类型，会在释放时自动置位nullptr
            m_messageWindowInfo[i].messageWindow->setAttribute(Qt::WA_DeleteOnClose);
            m_messageWindowInfo[i].messageWindow->setIcon(icon);
            m_messageWindowInfo[i].messageWindow->setMessage(msg);
            m_messageWindowInfo[i].messageWindow->setDuration(duration);
            m_messageWindowInfo[i].messageWindow->installEventFilter(this);

            m_messageWindowInfo[i].messageWindow->show();
            break;
        }
    }
}

bool MessageWindowManager::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Show) {
        MessageWindow *messageWindow = qobject_cast<MessageWindow *>(watched);
        if (messageWindow) {
            for (int i = 0; i < MaxTipsWindowNum; i++) {
                if (m_messageWindowInfo[i].messageWindow == messageWindow) {
                    // 显示前窗口大小不固定，需要在显示后才能move窗口位置
                    QRect rt = m_messageWindowInfo[i].rect;
                    // 水平居中于屏幕，垂直位置已在 rect 中设置好
                    int xPos = rt.x() + (rt.width() - messageWindow->width()) / 2;
                    int yPos = rt.y();
                    m_messageWindowInfo[i].messageWindow->move(xPos, yPos);
                    break;
                }
            }
        }
    }

    return QObject::eventFilter(watched, event);
}

