#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QListView>

#include "clipboardmodel.h"
#include "dbusdisplay.h"

#include <DBlurEffectWidget>
DWIDGET_USE_NAMESPACE

class MainWindow : public DBlurEffectWidget
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void itemAdded(ItemData *item);
    void itemRemoved(ItemData *item);

    void geometryChanged();

protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    void initConnect();

private:
    DBusDisplay *m_displayInter;

    QListView m_listview;
    ClipboardModel *m_model;
    QRect m_geometry;

};

#endif // MAINWINDOW_H
