#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QListView>

#include "clipboardmodel.h"
#include "dbusdisplay.h"
#include "itemdelegate.h"
#include "constants.h"
#include "dbusdock.h"

#include <DBlurEffectWidget>
#include <DWindowManagerHelper>

DWIDGET_USE_NAMESPACE
DGUI_USE_NAMESPACE
class QPushButton;
class MainWindow : public DBlurEffectWidget
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

public Q_SLOTS:
    void Toggle();

public slots:
    void geometryChanged();

protected:
    void mouseMoveEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *e) override;

private:
    void initUI();
    void initConnect();

private:
    DBusDisplay *m_displayInter;

    QPushButton *m_clearButton;
    QListView *m_listview;
    ClipboardModel *m_model;
    ItemDelegate *m_itemDelegate;

    DBusDock *m_dockInter;
};

#endif // MAINWINDOW_H
