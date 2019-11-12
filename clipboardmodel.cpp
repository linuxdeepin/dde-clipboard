#include "clipboardmodel.h"

#include <QApplication>
#include <QPointer>
#include <QDebug>

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

void ClipboardModel::removeData(ItemData *data)
{
    int row = m_data.indexOf(data);

    beginRemoveRows(QModelIndex(), row, row);
    auto item = m_data.takeAt(m_data.indexOf(data));
    endRemoveRows();

    item->deleteLater();
}

void ClipboardModel::extract(ItemData *data)
{
    int idx = m_data.indexOf(data);
    if (idx < 1)
        return;

    beginRemoveRows(QModelIndex(), idx, idx);
    m_data.removeOne(data);
    endRemoveRows();

    QMimeData *mimeData = new QMimeData;
    switch (data->type()) {
    case ItemData::Text:
        mimeData->setText(data->text());
        mimeData->setHtml(data->html().isEmpty() ? data->text() : data->html());
        break;
    case ItemData::Image:
        mimeData->setImageData(data->pixmap());
        break;
    case ItemData::File:
        mimeData->setUrls(data->urls());
        break;
    }

    QMapIterator<QString, QByteArray> it(data->formatMap());
    while (it.hasNext()) {
        it.next();
        mimeData->setData(it.key(), it.value());
    }

    m_board->setMimeData(mimeData);

    data->deleteLater();
}

void ClipboardModel::clipDataChanged()
{
    beginInsertRows(QModelIndex(), 0, 0);

    const QMimeData *mimeData = m_board->mimeData();
    ItemData *item = new ItemData(mimeData);

    connect(item, &ItemData::distory, this, &ClipboardModel::removeData);
    connect(item, &ItemData::reborn, this, &ClipboardModel::extract);

    m_data.push_front(item);

    endInsertRows();
}
