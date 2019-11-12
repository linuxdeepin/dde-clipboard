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
class ItemTitle;
class ClipboardModel;
class PixmapLabel;
//可调节背景透明度的圆角widget
class AlphaWidget : public DWidget
{
public:
    AlphaWidget(QWidget *parent = nullptr);

    void setAlpha(int alpha);

    int hoverAlpha() {return m_hoverAlpha;}
    void setHoverAlpha(int alpha) {m_hoverAlpha = alpha; update();}

    int unHoverAlpha() {return m_unHoverAlpha;}
    void setUnHoverAlpha(int alpha) {m_unHoverAlpha = alpha; update();}

    int radius() {return m_radius;}
    void setRadius(int radius);

private:
    int m_radius = 0;
    int m_hoverAlpha = 0;
    int m_unHoverAlpha = 0;
    bool m_havor = false;

protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void enterEvent(QEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;
};

//标题区域
class ItemTitle : public DWidget
{
    Q_OBJECT
public:
    ItemTitle(QWidget *parent = nullptr);

    void setDataName(const QString &text);
    void setIcon(const QIcon &icon);
    void setCreateTime(const QDateTime &time);

Q_SIGNALS:
    void close();

public Q_SLOTS:
    void onHoverStateChanged(bool hover);

private Q_SLOTS:
    void onRefreshTime();

private:
    QString CreateTimeString(const QDateTime &time);

private:
    DIconButton *m_icon = nullptr;
    DLabel *m_nameLabel = nullptr;
    DLabel *m_timeLabel = nullptr;
    QTimer *m_refreshTimer = nullptr;
    DIconButton *m_closeButton = nullptr;

    QDateTime m_time;
};

class ItemWidget : public AlphaWidget
{
    Q_OBJECT
public:
    ItemWidget(ClipboardModel *model, QPointer<ItemData> data, QWidget *parent = nullptr);

Q_SIGNALS:
    void clicked(bool checked = false);
    void popData(QPointer<ItemData> data);
    void hoverStateChanged(bool);

private:
    void initUI();
    void initStyle(QPointer<ItemData> data);

    void setText(const QString &text, const QString &length);
    void setPixmap(const QPixmap &pixmap);
    void setFilePixmap(const QPixmap &pixmap);
    void setPixmaps(const QList<QPixmap> &list);
    void setDataName(const QString &text);
    void setIcon(const QIcon &icon);
    void setCreateTime(const QDateTime &time);

private:
    ClipboardModel *m_model = nullptr;
    QPointer<ItemData> m_data;

    ItemTitle *m_titleWidget = nullptr;
    /*Dtk::Widget::DLabel*/PixmapLabel *m_contentLabel = nullptr;
    Dtk::Widget::DLabel *m_statusLabel = nullptr;
    QVBoxLayout *m_layout = nullptr;

    QString m_text;
    QPixmap m_pixmap;
    QList<QUrl> m_urls;

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void enterEvent(QEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;
    virtual void focusInEvent(QFocusEvent *event) override;
    virtual void focusOutEvent(QFocusEvent *event) override;
};
#endif // ITEMBASEWIDGET_H
