#include "pixmaplabel.h"
#include "constants.h"

#include <QPainter>
#include <QDebug>

PixmapLabel::PixmapLabel(const QList<QPixmap> &list, QWidget *parent)
    : DLabel(parent)
    , m_pixmapList(list)
{

}

PixmapLabel::PixmapLabel(QWidget *parent)
    : DLabel(parent)
{

}

void PixmapLabel::setPixmapList(const QList<QPixmap> &list)
{
    m_pixmapList = list;

    update();
}

QSize PixmapLabel::minimumSizeHint() const
{
    return QSize(128, 128);
}

void PixmapLabel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(Qt::transparent);

    painter.drawRoundedRect(rect(), 8, 8);

    qreal scale = 1.0;

    if (m_pixmapList.size() == 1) {
        QPixmap pix = m_pixmapList[0];
        if (pix.size() == QSize(0, 0))
            return;
        if (pix.width() >= pix.height()) {
            scale = pix.width() * 1.0 / PixmapWidth;
        } else {
            scale = pix.height() * 1.0 / PixmapHeight;
        }

        int x = int(width() - pix.size().width() / scale) / 2;
        int y = int(height() - pix.size().height() / scale) / 2;

        painter.drawPixmap(x, y, pix.scaled(pix.size() / scale, Qt::KeepAspectRatio));
    } else {
        for (int i = 0 ; i < m_pixmapList.size(); ++i) {
            QPixmap pix = m_pixmapList[i];
            if (pix.size() == QSize(0, 0))
                continue;
            if (pix.width() >= pix.height()) {
                scale = pix.width() * 1.0 / PixmapWidth;
            } else {
                scale = pix.height() * 1.0 / PixmapHeight;
            }
            int x = int(width() - pix.size().width() / scale) / 2 + (i - 1) * PixmapMargin;
            int y = int(height() - pix.size().height() / scale) / 2 + (i - 1) * PixmapMargin;

            painter.drawPixmap(x, y, pix.scaled(pix.size() / scale, Qt::KeepAspectRatio));
        }
    }

    return DLabel::paintEvent(event);
}
