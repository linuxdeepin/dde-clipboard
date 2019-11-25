#ifndef ICONBUTTON_H
#define ICONBUTTON_H
#include <DPushButton>

DWIDGET_USE_NAMESPACE

class IconButton : public DPushButton
{
public:
    IconButton(QWidget *parent = nullptr);

    void paintEvent(QPaintEvent *event);

    void setFocusState(bool has);

private:
    bool m_hasFocus = false;
};

#endif // ICONBUTTON_
