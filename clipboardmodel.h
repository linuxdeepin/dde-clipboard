#ifndef CLIPBOARDMODEL_H
#define CLIPBOARDMODEL_H

#include <QClipboard>
#include <QObject>

#include "itemdata.h"

class ClipboardModel : public QObject
{
    Q_OBJECT
public:
    explicit ClipboardModel(QObject *parent = nullptr);

    // 清空所有剪贴板
    void clear();

    // 删除
    void removeItem(ItemData *item);

    // 提取
    void extract(ItemData *item);

    // all data
    const QList<ItemData *> data() const;

signals:
    void dataAdded(ItemData *item);
    void dataRemoved(ItemData *item);

public slots:
    void clipDataChanged();

private:
    QList<ItemData *> m_data;
    QClipboard *m_board;
};

#endif // CLIPBOARDMODEL_H
