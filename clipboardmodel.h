#ifndef CLIPBOARDMODEL_H
#define CLIPBOARDMODEL_H

#include <QAbstractListModel>
#include <QClipboard>
#include <QListView>

#include "itemdata.h"

class ClipboardModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit ClipboardModel(QListView *list, QObject *parent = nullptr);

    // 清空所有剪贴板
    void clear();

    const QList<ItemData *> data();

Q_SIGNALS:
    void dataAdded();
    void dataRemoved();
    void dataAllCleared();

protected:
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    // 删除
    void removeData(ItemData *data);

    // 提取到第一个
    void extract(ItemData *data);

    bool isDataValid(const QMimeData *data);

protected slots:
    void clipDataChanged();

private:
    QList<ItemData *> m_data;
    QClipboard *m_board;
    QListView *m_list;
};

#endif // CLIPBOARDMODEL_H
