#ifndef PIXMAPLABEL_H
#define PIXMAPLABEL_H
#include <DLabel>

#include <QTextOption>

DWIDGET_USE_NAMESPACE

class QTextLayout;
class PixmapLabel : public DLabel
{
public:
    PixmapLabel(const QList<QPixmap> &list, QWidget *parent = nullptr);
    PixmapLabel(QWidget *parent = nullptr);

    const QString &text() {return m_text;}

    void setText(const QString &text);
    void setPixmapList(const QList<QPixmap> &list);

    virtual QSize minimumSizeHint() const override;
    virtual QSize sizeHint() const override;

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

protected:
    virtual void paintEvent(QPaintEvent *event) override;
};

#endif // PIXMAPLABEL_H
