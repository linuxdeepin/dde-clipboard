#ifndef ICONBUTTON_H
#define ICONBUTTON_H
#include <DWidget>

DWIDGET_USE_NAMESPACE

class IconButton : public DWidget
{
    Q_OBJECT
public:
    explicit IconButton(QWidget *parent = nullptr);
    explicit IconButton(const QString &text, QWidget *parent = nullptr);

    void setText(const QString &text);
    void setFocusState(bool has);
    void setBackOpacity(int opacity);
    void setRadius(int radius);

Q_SIGNALS:
    void clicked();

private:
    QString m_text;
    bool m_hasFocus;
    bool m_hover;
    int m_opacity;
    int m_radius;
    bool m_hasBackColor;

protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void enterEvent(QEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;
};

#endif // ICONBUTTON_
