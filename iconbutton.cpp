#include "iconbutton.h"

#include <QPainter>

IconButton::IconButton(QWidget *parent)
    : DPushButton(parent)
{

}

void IconButton::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);

    QColor c = palette().color(QPalette::WindowText);
    c.setAlpha(m_hasFocus ? 80 : 0);

    painter.setPen(Qt::NoPen);
    painter.setBrush(c);
    painter.drawRoundRect(rect(), 100, 100);
    return DPushButton::paintEvent(event);
}

void IconButton::setFocusState(bool has)
{
    m_hasFocus = has;
    update();
}
