// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
