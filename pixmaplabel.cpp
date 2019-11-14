#include "pixmaplabel.h"
#include "constants.h"

#include <QPainter>
#include <QDebug>
#include <QBitmap>
#include <QTextLayout>
#include <QTextOption>
#include <QTextLine>
#include <QVBoxLayout>

#include <QtGui/private/qtextengine_p.h>

PixmapLabel::PixmapLabel(const QList<QPixmap> &list, QWidget *parent)
    : DLabel(parent)
    , m_pixmapList(list)
{

}

PixmapLabel::PixmapLabel(QWidget *parent)
    : DLabel(parent)
{

}

void PixmapLabel::setText(const QString &text)
{
    m_text = text;

    update();
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
    return QSize(ItemWidth - ContentMargin * 2, ItemHeight - TitleHeight);
}

void PixmapLabel::elideText(QTextLayout *layout, const QSizeF &size, QTextOption::WrapMode wordWrap,
                            Qt::TextElideMode mode, qreal lineHeight, int flags, QStringList *lines,
                            QPainter *painter, QPointF offset, const QColor &shadowColor, const QPointF &shadowOffset,
                            const QBrush &background, qreal backgroundRadius, QList<QRectF> *boundingRegion)
{
    qreal height = 0;
    bool drawBackground = background.style() != Qt::NoBrush;
    bool drawShadow = shadowColor.isValid();

    QString text = layout->engine()->hasFormats() ? layout->engine()->block.text() : layout->text();
    QTextOption &text_option = *const_cast<QTextOption *>(&layout->textOption());

    text_option.setWrapMode(wordWrap);

    if (flags & Qt::AlignRight)
        text_option.setAlignment(Qt::AlignRight);
    else if (flags & Qt::AlignHCenter)
        text_option.setAlignment(Qt::AlignHCenter);

    if (painter) {
        text_option.setTextDirection(painter->layoutDirection());
        layout->setFont(painter->font());
    } else {
        // dont paint
        layout->engine()->ignoreBidi = true;
    }

    auto naturalTextRect = [&](const QRectF rect) {
        QRectF new_rect = rect;

        new_rect.setHeight(lineHeight);

        return new_rect;
    };

    auto drawShadowFun = [&](const QTextLine & line) {
        const QPen pen = painter->pen();

        painter->setPen(shadowColor);
        line.draw(painter, shadowOffset);

        // restore
        painter->setPen(pen);
    };

    layout->beginLayout();

    QTextLine line = layout->createLine();
    QRectF lastLineRect;

    while (line.isValid()) {
        height += lineHeight;

        if (height + lineHeight > size.height()) {
            const QString &end_str = layout->engine()->elidedText(mode, qRound(size.width()), flags, line.textStart());

            layout->endLayout();
            layout->setText(end_str);

            if (layout->engine()->block.docHandle()) {
                const_cast<QTextDocument *>(layout->engine()->block.document())->setPlainText(end_str);
            }

            text_option.setWrapMode(QTextOption::NoWrap);
            layout->beginLayout();
            line = layout->createLine();
            line.setLineWidth(size.width() - 1);
            text = end_str;
        } else {
            line.setLineWidth(size.width());
        }

        line.setPosition(offset);

        const QRectF rect = naturalTextRect(line.naturalTextRect());

        if (painter) {
            if (drawBackground) {
                const QMarginsF margins(backgroundRadius, 0, backgroundRadius, 0);
                QRectF backBounding = rect;
                QPainterPath path;

                if (lastLineRect.isValid()) {
                    if (qAbs(rect.width() - lastLineRect.width()) < backgroundRadius * 2) {
                        backBounding.setWidth(lastLineRect.width());
                        backBounding.moveCenter(rect.center());
                        path.moveTo(lastLineRect.x() - backgroundRadius, lastLineRect.bottom() - backgroundRadius);
                        path.lineTo(lastLineRect.x(), lastLineRect.bottom() - 1);
                        path.lineTo(lastLineRect.right(), lastLineRect.bottom() - 1);
                        path.lineTo(lastLineRect.right() + backgroundRadius, lastLineRect.bottom() - backgroundRadius);
                        path.lineTo(lastLineRect.right() + backgroundRadius, backBounding.bottom() - backgroundRadius);
                        path.arcTo(backBounding.right() - backgroundRadius, backBounding.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 0, -90);
                        path.lineTo(backBounding.x(), backBounding.bottom());
                        path.arcTo(backBounding.x() - backgroundRadius, backBounding.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 270, -90);
                        lastLineRect = backBounding;
                    } else if (lastLineRect.width() > rect.width()) {
                        backBounding += margins;
                        path.moveTo(backBounding.x() - backgroundRadius, backBounding.y() - 1);
                        path.arcTo(backBounding.x() - backgroundRadius * 2, backBounding.y() - 1, backgroundRadius * 2, backgroundRadius * 2 + 1, 90, -90);
                        path.lineTo(backBounding.x(), backBounding.bottom() - backgroundRadius);
                        path.arcTo(backBounding.x(), backBounding.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 180, 90);
                        path.lineTo(backBounding.right() - backgroundRadius, backBounding.bottom());
                        path.arcTo(backBounding.right() - backgroundRadius * 2, backBounding.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 270, 90);
                        path.lineTo(backBounding.right(), backBounding.top() + backgroundRadius);
                        path.arcTo(backBounding.right(), backBounding.top() - 1, backgroundRadius * 2, backgroundRadius * 2 + 1, 180, -90);
                        path.closeSubpath();
                        lastLineRect = rect;
                    } else {
                        backBounding += margins;
                        path.moveTo(lastLineRect.x() - backgroundRadius * 2, lastLineRect.bottom());
                        path.arcTo(lastLineRect.x() - backgroundRadius * 3, lastLineRect.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 270, 90);
                        path.lineTo(lastLineRect.x(), lastLineRect.bottom() - 1);
                        path.lineTo(lastLineRect.right(), lastLineRect.bottom() - 1);
                        path.lineTo(lastLineRect.right() + backgroundRadius, lastLineRect.bottom() - backgroundRadius * 2);
                        path.arcTo(lastLineRect.right() + backgroundRadius, lastLineRect.bottom() - backgroundRadius * 2, backgroundRadius * 2, backgroundRadius * 2, 180, 90);

//                        path.arcTo(lastLineRect.x() - backgroundReaius, lastLineRect.bottom() - backgroundReaius * 2, backgroundReaius * 2, backgroundReaius * 2, 180, 90);
//                        path.lineTo(lastLineRect.x() - backgroundReaius * 3, lastLineRect.bottom());
//                        path.moveTo(lastLineRect.right(), lastLineRect.bottom());
//                        path.arcTo(lastLineRect.right() - backgroundReaius, lastLineRect.bottom() - backgroundReaius * 2, backgroundReaius * 2, backgroundReaius * 2, 270, 90);
//                        path.arcTo(lastLineRect.right() + backgroundReaius, lastLineRect.bottom() - backgroundReaius * 2, backgroundReaius * 2, backgroundReaius * 2, 180, 90);
//                        path.lineTo(lastLineRect.right(), lastLineRect.bottom());

                        path.addRoundedRect(backBounding, backgroundRadius, backgroundRadius);
                        lastLineRect = rect;
                    }
                } else {
                    lastLineRect = backBounding;
                    path.addRoundedRect(backBounding + margins, backgroundRadius, backgroundRadius);
                }

                bool a = painter->testRenderHint(QPainter::Antialiasing);
                qreal o = painter->opacity();

                painter->setRenderHint(QPainter::Antialiasing, true);
                painter->setOpacity(1);
                painter->fillPath(path, background);
                painter->setRenderHint(QPainter::Antialiasing, a);
                painter->setOpacity(o);
            }

            if (drawShadow) {
                drawShadowFun(line);
            }

            line.draw(painter, QPointF(0, 0));
        }

        if (boundingRegion) {
            boundingRegion->append(rect);
        }

        offset.setY(offset.y() + lineHeight);

//        // find '\n'
//        int text_length_line = line.textLength();
//        for (int start = line.textStart(); start < line.textStart() + text_length_line; ++start) {
//            if (text.at(start) == '\n')
//                height += lineHeight;
//        }

        if (lines) {
            lines->append(text.mid(line.textStart(), line.textLength()));
        }

        if (height + lineHeight > size.height())
            break;

        line = layout->createLine();
    }

    layout->endLayout();
}

QString PixmapLabel::elideText(const QString &text, const QSizeF &size,
                               QTextOption::WrapMode wordWrap, const QFont &font,
                               Qt::TextElideMode mode, qreal lineHeight, qreal flags)
{
    QTextLayout textLayout(text);

    textLayout.setFont(font);

    QStringList lines;

    elideText(&textLayout, size, wordWrap, mode, lineHeight, flags, &lines);

    return lines.join('\n');
}


void PixmapLabel::paintEvent(QPaintEvent *event)
{
    qDebug() << __PRETTY_FUNCTION__;

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

    //draw lines
    if (!m_text.isEmpty()) {
        int lineNum = 4;
        int lineHeight = (height() - TextContentTopOffset) / 4;
        for (int i  = 0 ; i < lineNum; ++i) {
            QPoint start(0, (i + 1)*lineHeight + TextContentTopOffset);
            QPoint end(width(), (i + 1)*lineHeight + TextContentTopOffset);
            painter.setPen(QPen(palette().color(QPalette::Shadow), 2));
            painter.drawLine(start, end);
        }

        //drawText
        QString t = elideText(m_text, size(), QTextOption::WrapAtWordBoundaryOrAnywhere, font(), Qt::ElideMiddle, 0);
        QStringList labelTexts = t.split("\n");

        static const int maxLineCount = 4;
        int textIndex = 0;
        for (int rectIndex = 0; textIndex < labelTexts.length(); rectIndex++, textIndex++) {
            if (textIndex > (maxLineCount - 1)) {
                break;
            }
            QRect textRect(0, rectIndex * lineHeight + TextContentTopOffset, width(), lineHeight);
            QTextOption option;
            option.setAlignment(Qt::AlignBottom);
            painter.setPen(palette().color(QPalette::Text));
            if (labelTexts.at(textIndex).simplified().isEmpty()) {
                textIndex ++ ;
                if (textIndex >= labelTexts.length() || textIndex > (maxLineCount - 1)) {
                    break;
                }
            }
            painter.drawText(textRect, labelTexts.at(textIndex).trimmed(), option);
        }
    }
    return DLabel::paintEvent(event);
}
