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

    Q_EMIT dataAllCleared();
}

const QList<ItemData *> ClipboardModel::data()
{
    return m_data;
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

    Q_EMIT dataRemoved();
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

    QMapIterator<QString, QByteArray> it(data->formatMap());
    while (it.hasNext()) {
        it.next();
        mimeData->setData(it.key(), it.value());
    }

    switch (data->type()) {
    case ItemData::Text:
        mimeData->setText(data->text());
        mimeData->setHtml(data->html().isEmpty() ? data->text() : data->html());
        break;
    case ItemData::Image:
        mimeData->setImageData(data->imageData());
        break;
    case ItemData::File:
        mimeData->setUrls(data->urls());
        break;
    }

    m_board->setMimeData(mimeData);

    data->deleteLater();
}

bool ClipboardModel::isDataValid(const QMimeData *data)
{
    //文本全是空格的情况需要过滤
    if (!data->hasText()
            && !data->hasUrls()
            && !data->hasHtml()
            && !data->hasColor()
            && !data->hasImage()
            && data->formats().size() == 0)
        return false;

    // -- 1有些程序不规范，复制空内容也写入了剪贴板
    if (data->hasHtml() && data->html().simplified().isEmpty())
        return false;

    // -- 2
    if (data->hasText() && data->text().simplified().isEmpty())
        return false;

    // -- 3
    if (data->hasImage()) {
        QPixmap pix = qvariant_cast<QPixmap>(data->imageData());
        if (pix.size() == QSize(0, 0)) {
            return false;
        }
    }

    // -- 4
    if (data->hasUrls() && data->urls().size() == 0)
        return false;

    return true;
}

void ClipboardModel::clipDataChanged()
{
    const QMimeData *mimeData = m_board->mimeData();
#if 0
    qDebug() << "hasUrls:" << mimeData->hasUrls();
    qDebug() << "hasText:" << mimeData->hasText();
    qDebug() << "hasHtml:" << mimeData->hasHtml();
    qDebug() << "hasImage:" << mimeData->hasImage();
    qDebug() << "hasColor:" << mimeData->hasColor();
    qDebug() << "imageData type:" << mimeData->imageData().type();
#endif
    if (!isDataValid(mimeData)) {
        qDebug() << "data not valid";
        return;
    }

    beginInsertRows(QModelIndex(), 0, 0);

    ItemData *item = new ItemData(mimeData);
    connect(item, &ItemData::distory, this, &ClipboardModel::removeData);
    connect(item, &ItemData::reborn, this, &ClipboardModel::extract);
    m_data.push_front(item);

    endInsertRows();

    Q_EMIT dataAdded();
}
