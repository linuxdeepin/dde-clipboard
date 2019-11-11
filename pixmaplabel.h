#ifndef PIXMAPLABEL_H
#define PIXMAPLABEL_H
#include <DLabel>

DWIDGET_USE_NAMESPACE

class PixmapLabel : public DLabel
{
public:
    PixmapLabel(const QList<QPixmap> &list, QWidget *parent = nullptr);
    PixmapLabel(QWidget *parent = nullptr);

    void setPixmapList(const QList<QPixmap> &list);

    virtual QSize  minimumSizeHint() const override;

private:
    QList<QPixmap> m_pixmapList;

protected:
    virtual void paintEvent(QPaintEvent *event) override;
};

#endif // PIXMAPLABEL_H
