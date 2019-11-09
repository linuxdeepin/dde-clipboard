#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QListView>

#include "clipboardmodel.h"
#include "dbusdisplay.h"
#include "itemdelegate.h"

#include <DBlurEffectWidget>
#include <DWindowManagerHelper>
DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE

class MainWindow : public DBlurEffectWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

public slots:
    void geometryChanged();

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    void initUI();
    void initConnect();

private:
    DBusDisplay *m_displayInter;

    QListView *m_listview;
    ClipboardModel *m_model;
    ItemDelegate *m_itemDelegate;

    DWindowManagerHelper *m_wmHelper;
    QVariantAnimation *m_showAni;
    QVariantAnimation *m_hideAni;
};

#endif // MAINWINDOW_H
