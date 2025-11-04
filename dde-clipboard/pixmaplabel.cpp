// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pixmaplabel.h"
#include "constants.h"

#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <QTextLayout>
#include <QTextOption>
#include <QTextLine>
#include <QVBoxLayout>
#include <QFontMetrics>
#include <DFontSizeManager>

PixmapLabel::PixmapLabel(QPointer<ItemData> data,QWidget *parent)
    : DLabel(parent)
    , m_istext(false)
    , m_data(data)
{
    // QFont font = DFontSizeManager::instance()->t8();
    // setFont(font);
}

/*!
 */
void PixmapLabel::setText(bool is_text)
{
    m_istext = is_text; // it label is text
}

/*!
 * \~chinese \name setPixmapList
 * \~chinese \brief 设置剪切板中显示的内容
 * \~chinese \param list 存放图片的容器
 */
void PixmapLabel::setPixmapList(const QList<QPixmap> &list)
{
    m_pixmapList = list;
}

/*!
 * \~chinese \name minimumSizeHint
 * \~chinese \brief 推荐显示的最小大小(宽度为180,高度为100)
 */
QSize PixmapLabel::minimumSizeHint() const
{
    return QSize(FileIconWidth, FileIconHeight);
}

/*!
 * \~chinese \name sizeHint
 * \~chinese \brief 推荐显示的大小
 */
QSize PixmapLabel::sizeHint() const
{
    int textAreaWidth = getTextAreaWidth();
    
    if (m_istext && !m_data.isNull()) {
        int lineCount = calculateTextLineCount();
        int fontHeight = fontMetrics().height();
        int totalHeight = TextContentTopMargin + lineCount * fontHeight + (lineCount - 1) * TextLineSpacing;
        return QSize(textAreaWidth, totalHeight);
    }
    
    return QSize(textAreaWidth, ItemHeight - ItemTitleHeight);
}

bool PixmapLabel::calculateTextLines(const QFontMetrics &fm, QStringList &outLines) const
{
    outLines.clear();
    
    if (!m_istext || m_data.isNull()) {
        return false;
    }

    QString originalText = m_data->text();
    originalText.replace("\n", " ");
    
    const int textAreaWidth = getTextAreaWidth();
    const int ellipsisWidth = fm.horizontalAdvance(ELLIPSIS);
    
    QString remainingText = originalText;
    while (!remainingText.isEmpty() && outLines.size() < MAX_TEXT_LINES) {
        const bool isLastLine = (outLines.size() == MAX_TEXT_LINES - 1);
        int currentLineWidth = textAreaWidth;
        
        if (isLastLine && fm.horizontalAdvance(remainingText) > textAreaWidth) {
            currentLineWidth = textAreaWidth - ellipsisWidth;
        }
        
        int currentWidth = 0;
        int index = 0;
        while (index < remainingText.length()) {
            QChar ch = remainingText.at(index);
            int charWidth = fm.horizontalAdvance(ch);
            
            if (currentWidth + charWidth > currentLineWidth) {
                if (index == 0) {
                    index = 1; // 至少保证一个字符
                }
                break;
            }
            
            currentWidth += charWidth;
            index++;
        }
        
        outLines.push_back(remainingText.left(index));
        remainingText.remove(0, index);
    }
    
    return !remainingText.isEmpty();
}

int PixmapLabel::calculateTextLineCount() const
{
    if (!m_istext || m_data.isNull()) {
        return 0;
    }

    QStringList lines;
    QFontMetrics fm = fontMetrics();
    calculateTextLines(fm, lines);
    return lines.size();
}

void PixmapLabel::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::transparent);

    QStyle *style = QWidget::style();
    QStyleOption opt;
    opt.initFrom(this);

    //drawPixmaps
    if (m_pixmapList.size() == 1) {
        QPixmap pix = m_pixmapList[0];
        qreal scale = Globals::GetScale(pix.size(), FileIconWidth, FileIconHeight);
        int x = int(width() - pix.size().width() / scale) / 2;
        int y = int(height() - pix.size().height() / scale) / 2;

        if (!isEnabled())
            pix = style->generatedIconPixmap(QIcon::Disabled, pix, &opt);
        QPixmap newPix = pix.scaled(pix.size() / scale, Qt::KeepAspectRatio);
        style->drawItemPixmap(&painter, QRect(QPoint(x, y), newPix.size()), Qt::AlignCenter, newPix);
    } else {
        for (int i = 0 ; i < m_pixmapList.size(); ++i) {
            QPixmap pix = m_pixmapList[i];
            if (pix.size() == QSize(0, 0))
                continue;
            int x = 0;
            int y = 0;
            qreal scale = Globals::GetScale(pix.size(), FileIconWidth, FileIconHeight);
            if (!(m_pixmapList.size() % 2)) {//奇数个和偶数个计算方法不一样
                x = int(width() - (pix.size().width() / scale + PixmapxStep)) / 2 + i * PixmapxStep;
                y = int(height() - (pix.size().height() / scale + PixmapyStep)) / 2 + i * PixmapyStep;
            } else {
                x = int(width() - pix.size().width() / scale) / 2 + (i - 1) * PixmapxStep;
                y = int(height() - pix.size().height() / scale) / 2 + (i - 1) * PixmapyStep;
            }

            if (!isEnabled())
                pix = style->generatedIconPixmap(QIcon::Disabled, pix, &opt);
            QPixmap newPix = pix.scaled(pix.size() / scale, Qt::KeepAspectRatio);
            style->drawItemPixmap(&painter, QRect(QPoint(x, y), newPix.size()), Qt::AlignCenter, newPix);
        }
    }

    //draw lines
    if (m_istext) {
        const QFontMetrics fm = fontMetrics();
        QStringList labelTexts;
        
        const bool hasMoreText = calculateTextLines(fm, labelTexts);
        
        const int lineNum = labelTexts.length();
        const int fontHeight = fm.height();
        const int textAreaWidth = getTextAreaWidth();
        
        for (int i = 0; i < lineNum; ++i) {
            const int lineY = TextContentTopMargin + (i + 1) * fontHeight + i * TextLineSpacing;
            painter.setPen(QPen(palette().color(QPalette::Shadow), 2));
            painter.drawLine(QPoint(0, lineY), QPoint(width(), lineY));
        }

        // 绘制文本
        QTextOption option;
        option.setAlignment(Qt::AlignBottom);
        option.setWrapMode(QTextOption::NoWrap);
        painter.setPen(palette().color(QPalette::Text));
        
        for (int i = 0; i < lineNum; i++) {
            const int rectY = TextContentTopMargin + i * (fontHeight + TextLineSpacing);
            const QRect textRect(0, rectY, textAreaWidth, fontHeight);
            
            QString textToDraw = labelTexts.at(i);
            
            if (i == lineNum - 1 && lineNum == MAX_TEXT_LINES && hasMoreText) {
                textToDraw += ELLIPSIS;
            }
            
            painter.drawText(textRect, textToDraw, option);
        }
    }
    return DLabel::paintEvent(event);
}

void PixmapLabel::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::FontChange) {
        updateGeometry();
        update();
    }
    return DLabel::changeEvent(event);
}
