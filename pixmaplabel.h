/*
 * Copyright (C) 2018 ~ 2025 Deepin Technology Co., Ltd.
 *
 * Author:     fanpengcheng <fanpengcheng_cm@deepin.com>
 *
 * Maintainer: fanpengcheng <fanpengcheng_cm@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef PIXMAPLABEL_H
#define PIXMAPLABEL_H
#include <DLabel>

#include <QTextOption>

DWIDGET_USE_NAMESPACE

class QTextLayout;
/*!
 * \~chinese \class PixmapLabel
 * \~chinese \brief 继承于DLabel,DLabel继承于QLabel,用于显示剪切块中的文字和图标等信息
 */
class PixmapLabel : public DLabel
{
public:
    PixmapLabel(const QList<QPixmap> &list, QWidget *parent = nullptr);
    PixmapLabel(QWidget *parent = nullptr);

    /*!
     * \~chinese \name text
     * \~chinese \brief 获取剪切板中的文字
     * \~chinese \return 返回剪切板中的文字
     */
    const QString &text() {return m_text;}

    /*!
     * \~chinese \name setText
     * \~chinese \brief 设置剪切板中的文字
     * \~chinese \param text 剪切板中需要显示的文字
     */
    void setText(const QString &text);
    /*!
     * \~chinese \name setPixmapList
     * \~chinese \brief 设置剪切板中显示的内容
     * \~chinese \param list 存放图片的容器
     */
    void setPixmapList(const QList<QPixmap> &list);

    /*!
     * \~chinese \name minimumSizeHint
     * \~chinese \brief 推荐显示的最小大小(宽度为180,高度为100)
     */
    virtual QSize minimumSizeHint() const override;
    /*!
     * \~chinese \name sizeHint
     * \~chinese \brief 推荐显示的大小
     */
    virtual QSize sizeHint() const override;

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
    QString elideText(const QString &text, const QSizeF &size,
                      QTextOption::WrapMode wordWrap,
                      const QFont &font,
                      Qt::TextElideMode mode,
                      qreal lineHeight,
                      qreal flags = 0);

    void elideText(QTextLayout *layout, const QSizeF &size,
                   QTextOption::WrapMode wordWrap,
                   Qt::TextElideMode mode, qreal lineHeight,
                   int flags = 0, QStringList *lines = 0,
                   QPainter *painter = 0, QPointF offset = QPoint(0, 0),
                   const QColor &shadowColor = QColor(),
                   const QPointF &shadowOffset = QPointF(0, 1),
                   const QBrush &background = QBrush(Qt::NoBrush),
                   qreal backgroundReaius = 4,
                   QList<QRectF> *boundingRegion = 0);


private:
    QString m_text;
    QList<QPixmap> m_pixmapList;

private:
    /*!
     * \~chinese \name getNextValidString
     * \~chinese \brief 获取当前行需要显示的字符串
     * \~chinese \param 返回当前行数和字符串
     */
    QPair<QString, int> getNextValidString(const QStringList &list, int from);

protected:
    virtual void paintEvent(QPaintEvent *event) override;
};

#endif // PIXMAPLABEL_H
