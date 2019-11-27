#include "iconbutton.h"

#include <QPainter>

IconButton::IconButton(QWidget *parent)
    : DWidget(parent)
{

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

    QColor c = palette().color(QPalette::WindowText);
    c.setAlpha(m_hasFocus ? 80 : (m_hover ? 40 : 0));

    painter.setPen(Qt::NoPen);
    painter.setBrush(c);
    painter.drawRoundRect(rect(), 100, 100);

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

void IconButton::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    Q_EMIT clicked();
    return;
}

void IconButton::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_hover = true;

    update();
}

void IconButton::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    m_hover = false;

    update();
}
