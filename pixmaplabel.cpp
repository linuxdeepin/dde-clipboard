#include "pixmaplabel.h"
#include "constants.h"

#include <QPainter>
#include <QDebug>
#include <QBitmap>

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
    return QSize(FileIconWidth, FileIconHeight);
}

QSize PixmapLabel::sizeHint() const
{
    return QSize(ItemWidth - ContentMargin * 2, ItemHeight - TitleHeight - StatusBarHeight);
}

void PixmapLabel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(Qt::transparent);

    //drawPixmaps
    qreal scale = 1.0;

    if (m_pixmapList.size() == 1) {
        QPixmap pix = m_pixmapList[0];
        if (pix.size() != QSize(0, 0)) {
            if (pix.width() >= pix.height()) {
                scale = pix.width() * 1.0 / FileIconWidth;
            } else {
                scale = pix.height() * 1.0 / FileIconHeight;
            }

            int x = int(width() - pix.size().width() / scale) / 2;
            int y = int(height() - pix.size().height() / scale) / 2;

            painter.drawPixmap(x, y, pix.scaled(pix.size() / scale, Qt::KeepAspectRatio));
        }
    } else {
        for (int i = 0 ; i < m_pixmapList.size(); ++i) {
            QPixmap pix = m_pixmapList[i];
            if (pix.size() == QSize(0, 0))
                continue;
            if (pix.width() >= pix.height()) {
                scale = pix.width() * 1.0 / FileIconWidth;
            } else {
                scale = pix.height() * 1.0 / FileIconHeight;
            }
            int x = int(width() - pix.size().width() / scale) / 2 + (i - 1) * PixmapxStep;
            int y = int(height() - pix.size().height() / scale) / 2 + (i - 1) * PixmapyStep;

            painter.drawPixmap(x, y, pix.scaled(pix.size() / scale, Qt::KeepAspectRatio));
        }
    }

    return DLabel::paintEvent(event);
}
