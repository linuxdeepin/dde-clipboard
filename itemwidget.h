#ifndef ITEMBASEWIDGET_H
#define ITEMBASEWIDGET_H
#include <DWidget>
#include <DIconButton>
#include <DLabel>

#include <QDateTime>
#include <QPointer>

#include "itemdata.h"

DWIDGET_USE_NAMESPACE

class QTimer;
class QVBoxLayout;
class PixmapLabel;
class ItemWidget : public DWidget
{
    Q_OBJECT
public:
    ItemWidget(QPointer<ItemData> data, QWidget *parent = nullptr);

    void setText(const QString &text, const QString &length);
    void setPixmap(const QPixmap &pixmap);              //设置图像缩略图
    void setFilePixmap(const QPixmap &pixmap);          //单个文件
    void setFilePixmaps(const QList<QPixmap> &list);    //多个文件
    void setClipType(const QString &text);              //剪贴类型
    void setIcon(const QIcon &icon);
    void setCreateTime(const QDateTime &time);

    //--- set style
    void setAlpha(int alpha);

    int hoverAlpha() const;
    void setHoverAlpha(int alpha);

    int unHoverAlpha() const;
    void setUnHoverAlpha(int alpha);

    int radius() {return m_radius;}
    void setRadius(int radius);
    //--- end

Q_SIGNALS:
    void close();
    void hoverStateChanged(bool);

public Q_SLOTS:
    void onHoverStateChanged(bool hover);

private Q_SLOTS:
    void onRefreshTime();

private:
    void initUI();
    void initStyle(QPointer<ItemData> data);
    QString CreateTimeString(const QDateTime &time);

private:
    QPointer<ItemData> m_data;

    // title
    DIconButton *m_icon = nullptr;
    DLabel *m_nameLabel = nullptr;
    DLabel *m_timeLabel = nullptr;
    DIconButton *m_closeButton = nullptr;

    // content
    PixmapLabel *m_contentLabel = nullptr;
    Dtk::Widget::DLabel *m_statusLabel = nullptr;

    QTimer *m_refreshTimer = nullptr;
    QVBoxLayout *m_layout = nullptr;

    //--- data
    QDateTime m_time;
    QString m_text;
    QPixmap m_pixmap;
    QList<QUrl> m_urls;

    //--- set style
    int m_radius = 0;
    int m_hoverAlpha = 0;
    int m_unHoverAlpha = 0;
    bool m_havor = false;

protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void enterEvent(QEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;
    virtual void focusInEvent(QFocusEvent *event) override;
    virtual void focusOutEvent(QFocusEvent *event) override;
};
#endif // ITEMBASEWIDGET_H
