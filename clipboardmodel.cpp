#include "clipboardmodel.h"
#include <QApplication>
#include <QPointer>

ClipboardModel::ClipboardModel(QListView *list, QObject *parent) : QAbstractListModel(parent)
    , m_board(QApplication::clipboard())
    , m_list(list)
{
    // 剪切板是否有数据
    if (m_board->ownsClipboard()) {

    }

    connect(m_board, &QClipboard::dataChanged, this, &ClipboardModel::clipDataChanged);
}

void ClipboardModel::clear()
{
    beginResetModel();
    m_data.clear();
    endResetModel();
}

int ClipboardModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_data.size();
}

QVariant ClipboardModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(role)
    if (!index.isValid() && m_data.size() <= 0 && index.row() > m_data.size()) {
        return QVariant();
    }

    QPointer<ItemData> dt = m_data.at(index.row());
    return QVariant::fromValue(dt);
}

Qt::ItemFlags ClipboardModel::flags(const QModelIndex &index) const
{
    if (index.isValid()) {
        if (m_list != nullptr) m_list->openPersistentEditor(index);
        return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
    }
    return QAbstractListModel::flags(index);
}

void ClipboardModel::removeItem(QModelIndex index)
{
    beginRemoveRows(QModelIndex(), index.row(), index.row());
    auto item = m_data.takeAt(index.row());
    endRemoveRows();

    item->deleteLater();
}

void ClipboardModel::removeData(ItemData *data)
{
    int row = m_data.indexOf(data);

    beginRemoveRows(QModelIndex(), row, row);
    auto item = m_data.takeAt(m_data.indexOf(data));
    endRemoveRows();

    item->deleteLater();
}

void ClipboardModel::extract(QModelIndex index)
{
    beginMoveRows(QModelIndex(), index.row(), index.row(), QModelIndex(), 0);
    auto item = m_data.takeAt(index.row());
    m_data.push_front(item);
    endMoveRows();
}

const QList<ItemData *> ClipboardModel::data() const
{
    return m_data;
}

void ClipboardModel::clipDataChanged()
{
    beginInsertRows(QModelIndex(), 0, 0);

    const QMimeData *mimeData = m_board->mimeData();
    ItemData *item = new ItemData(mimeData);
    m_data.push_front(item);

    endInsertRows();
}

void ClipboardModel::onPopData(ItemData *data)
{
    int row = m_data.indexOf(data);
    if (row == 0)
        return;

    beginMoveRows(QModelIndex(), row, row, QModelIndex(), 0);
    auto item = m_data.takeAt(row);
    m_data.push_front(item);
    endMoveRows();

    QMimeData *mimeData = new QMimeData;
    switch (item->type()) {
    case ItemData::Text:
        mimeData->setText(item->text());
        mimeData->setHtml(item->text());
        break;
    case ItemData::Image:
        mimeData->setImageData(item->pixmap());
        break;
    case ItemData::File:
        mimeData->setUrls(item->urls());
        break;
    default:
        break;
    }

    m_board->setMimeData(mimeData);//会触发增加一次复制的内容
}
