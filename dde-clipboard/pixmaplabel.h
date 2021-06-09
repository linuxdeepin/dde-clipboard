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
    explicit PixmapLabel(QWidget *parent = nullptr);

    /*!
     * \~chinese \name text
     * \~chinese \brief 获取剪切板中的文字
     * \~chinese \return 返回剪切板中的文字
     */
    const QString &text() {return m_text;}
    void setText(const QString &text);

    inline const QList<QPixmap> pixmapList() { return m_pixmapList; }
    void setPixmapList(const QList<QPixmap> &list);

    virtual QSize minimumSizeHint() const override;
    virtual QSize sizeHint() const override;

    QString elideText(const QString &text, const QSizeF &size,
                      QTextOption::WrapMode wordWrap,
                      const QFont &font,
                      Qt::TextElideMode mode,
                      qreal lineHeight,
                      int flags = 0);

    void elideText(QTextLayout *layout, const QSizeF &size,
                   QTextOption::WrapMode wordWrap,
                   Qt::TextElideMode mode, qreal lineHeight,
                   int flags = 0, QStringList *lines = 0);
private:
    QString m_text;
    QList<QPixmap> m_pixmapList;

private:
    QPair<QString, int> getNextValidString(const QStringList &list, int from);

protected:
    virtual void paintEvent(QPaintEvent *event) override;
};

#endif // PIXMAPLABEL_H
