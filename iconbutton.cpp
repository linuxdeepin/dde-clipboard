#include "iconbutton.h"

#include <QPainter>

IconButton::IconButton(QWidget *parent)
    : DWidget(parent)
    , m_text("")
    , m_hasFocus(false)
    , m_hover(false)
    , m_opacity(0)
    , m_radius(8)
    , m_hasBackColor(false)
{

}

IconButton::IconButton(const QString &text, QWidget *parent)
    : IconButton(parent)
{
    setText(text);
}

void IconButton::setText(const QString &text)
{
    m_text = text;

    update();
}

void IconButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);

    QColor color;
    if (m_hasBackColor) {
        color = palette().color(QPalette::Base);
        color.setAlpha(m_hover ? m_opacity : (m_opacity / 2));
    } else {
        color = palette().color(QPalette::WindowText);
        color.setAlpha(m_hasFocus ? 80 : (m_hover ? 40 : 0));
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawRoundedRect(rect(), m_radius, m_radius);

    QTextOption option;
    option.setAlignment(Qt::AlignCenter);
    painter.setPen(palette().color(QPalette::WindowText));
    painter.drawText(rect(), m_text, option);
}

void IconButton::setFocusState(bool has)
{
    m_hasFocus = has;

    update();
}

void IconButton::setBackOpacity(int opacity)
{
    if (opacity < 0 || m_opacity > 255)
        return;

    m_opacity = opacity;

    m_hasBackColor = true;

    update();
}

void IconButton::setRadius(int radius)
{
    m_radius = radius;

    update();
}

void IconButton::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    Q_EMIT clicked();

    return DWidget::mousePressEvent(event);
}

void IconButton::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_hover = true;

    update();

    return DWidget::enterEvent(event);
}

void IconButton::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_hover = false;

    update();

    return DWidget::leaveEvent(event);
}
