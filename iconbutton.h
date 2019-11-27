#ifndef ICONBUTTON_H
#define ICONBUTTON_H
#include <DWidget>

DWIDGET_USE_NAMESPACE

class IconButton : public DWidget
{
    Q_OBJECT
public:
    IconButton(QWidget *parent = nullptr);

    void setText(const QString &text);

    void setFocusState(bool has);

Q_SIGNALS:
    void clicked();

private:
    QString m_text;
    bool m_hasFocus = false;
    bool m_hover = false;

protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void enterEvent(QEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;
};

#endif // ICONBUTTON_
