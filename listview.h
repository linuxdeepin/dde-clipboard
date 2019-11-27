#ifndef LISTVIEW_H
#define LISTVIEW_H
#include <QListView>

class ListView : public QListView
{
    Q_OBJECT
public:
    ListView(QWidget *parent = nullptr);

    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void enterEvent(QEvent *event) override;
    virtual void leaveEvent(QEvent *event) override;

Q_SIGNALS:
    void extract(const QModelIndex &index);
};

#endif // LISTVIEW_H
