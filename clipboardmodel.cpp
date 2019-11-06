#include "clipboardmodel.h"
#include <QApplication>

ClipboardModel::ClipboardModel(QObject *parent) : QObject(parent)
    , m_board(QApplication::clipboard())
{
    // 剪切板是否有数据
    if (m_board->ownsClipboard()) {

    }

    connect(m_board, &QClipboard::dataChanged, this, &ClipboardModel::clipDataChanged);
}

void ClipboardModel::removeItem(ItemData *item)
{
    m_data.removeOne(item);
    item->deleteLater();

    emit dataRemoved(item);
}

void ClipboardModel::extract(ItemData *item)
{
    m_data.removeOne(item);
    m_data.push_front(item);
}

const QList<ItemData *> ClipboardModel::data() const
{
    return m_data;
}

void ClipboardModel::clipDataChanged()
{
    const QMimeData *mimeData = m_board->mimeData();

    ItemData *item = new ItemData(mimeData);
    m_data.push_front(item);

    emit dataAdded(item);
}
