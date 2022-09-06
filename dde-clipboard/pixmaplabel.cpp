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

/*!
 * \~chinese \name setText
 * \~chinese \brief 设置剪切板中的文字
 * \~chinese \param text 剪切板中需要显示的文字
 */
void PixmapLabel::setText(const QString &text)
{
    m_text = text.left(1024);//减少elideText计算消耗

    update();
}

/*!
 * \~chinese \name setPixmapList
 * \~chinese \brief 设置剪切板中显示的内容
 * \~chinese \param list 存放图片的容器
 */
void PixmapLabel::setPixmapList(const QList<QPixmap> &list)
{
    m_pixmapList = list;

    update();
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
    return QSize(ItemWidth - ContentMargin * 2, ItemHeight - ItemTitleHeight);
}

void PixmapLabel::elideText(QTextLayout *layout, const QSizeF &size, QTextOption::WrapMode wordWrap,
                            Qt::TextElideMode mode, qreal lineHeight, int flags, QStringList *lines)
{
    qreal height = 0;
    QPointF offset(0, 0);

    QString text = layout->engine()->hasFormats() ? layout->engine()->block.text() : layout->text();
    QTextOption &text_option = *const_cast<QTextOption *>(&layout->textOption());

    text_option.setWrapMode(wordWrap);

    if (flags & Qt::AlignRight)
        text_option.setAlignment(Qt::AlignRight);
    else if (flags & Qt::AlignHCenter)
        text_option.setAlignment(Qt::AlignHCenter);

    // dont paint
    layout->engine()->ignoreBidi = true;
    layout->beginLayout();

    QTextLine line = layout->createLine();

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
        offset.setY(offset.y() + lineHeight);

        if (lines) {
            lines->append(text.mid(line.textStart(), line.textLength()));
        }

        if (height + lineHeight > size.height())
            break;

        line = layout->createLine();
    }

    layout->endLayout();
}

QPair<QString, int> PixmapLabel::getNextValidString(const QStringList &list, int from)
{
    if (from < 0 || from > list.size() - 1)
        return QPair<QString, int>("", list.size() - 1);

    for (int i = from; i < list.size(); ++i) {
        if (!list.at(i).trimmed().isEmpty()) {
            return QPair<QString, int>(list.at(i).trimmed(), i + 1);
        }
    }

    return QPair<QString, int>("", list.size() - 1);
}

/*!
 * \~chinese \name elideText
 * \~chinese \brief 将文本转换为一定的格式返回
 * \~chinese \param text 文本信息
 * \~chinese \param size 显示文字窗口的大小
 * \~chinese \param wordWrap 控制换行符出现的位置
 * \~chinese \param mode 控制省略文本中省略号“…”的位置
 * \~chinese \param font 字体大小
 * \~chinese \param lineHeight 字符的行高
 * \~chinese \param flags 控制字体的对齐方式
 * \~chinese \return 转换好格式的文本
 */
QString PixmapLabel::elideText(const QString &text, const QSizeF &size,
                               QTextOption::WrapMode wordWrap, const QFont &font,
                               Qt::TextElideMode mode, qreal lineHeight, int flags)
{
    QTextLayout textLayout(text);

    textLayout.setFont(font);

    QStringList lines;

    elideText(&textLayout, size, wordWrap, mode, lineHeight, flags, &lines);

    return lines.join('\n');
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
    if (!m_text.isEmpty()) {
        int lineNum = 4;
        int lineHeight = (height() - TextContentTopMargin) / 4;
        for (int i  = 0 ; i < lineNum; ++i) {
            QPoint start(0, (i + 1)*lineHeight + TextContentTopMargin);
            QPoint end(width(), (i + 1)*lineHeight + TextContentTopMargin);
            painter.setPen(QPen(palette().color(QPalette::Shadow), 2));
            painter.drawLine(start, end);
        }

        //drawText
        QString t = elideText(m_text.simplified(), size(), QTextOption::WrapAnywhere, font(), Qt::ElideMiddle, 0);
        QStringList labelTexts = t.split("\n");

        static const int maxLineCount = 4;
        int textIndex = 0;
        int lineFrom = 0;
        for (int rectIndex = 0; textIndex < labelTexts.length(); rectIndex++, textIndex++) {
            if (textIndex > (maxLineCount - 1)) {
                break;
            }
            QRect textRect(0, rectIndex * lineHeight + TextContentTopMargin, width(), lineHeight);
            QTextOption option;
            option.setAlignment(Qt::AlignBottom);
            option.setWrapMode(QTextOption::NoWrap);//设置文本不能换行
            painter.setPen(palette().color(QPalette::Text));

            QPair<QString, int> pair = getNextValidString(labelTexts, lineFrom);
            lineFrom = pair.second;
            QString str = pair.first.trimmed();
            if (lineFrom == maxLineCount) {
                str.replace(str.size() - 3, 3, "...");
            }
            if(rectIndex==(maxLineCount-1)){
                QString lastStr = pair.first.trimmed();
                pair = getNextValidString(labelTexts, lineFrom);
                lineFrom = pair.second;
                lastStr += pair.first.trimmed();
                painter.drawText(textRect,fontMetrics().elidedText(lastStr,Qt::ElideRight,width()-2),option);
            }
            else {
                painter.drawText(textRect, pair.first.trimmed(), option);
            }
        }
    }
    return DLabel::paintEvent(event);
}
