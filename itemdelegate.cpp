#include "itemdelegate.h"
#include "clipboardmodel.h"
#include "itemdata.h"

#include <QPointer>


ItemDelegate::ItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

QWidget *ItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    QPointer<ItemData> data = index.data().value<QPointer<ItemData>>();

    QWidget *w = new QWidget(parent);
    w->setStyleSheet("background-color:red");
    return w;
}

QSize ItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    return QSize(ITEM_WIDTH, ITEM_HEIGHT + ITEM_MARGIN);
}

void ItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    QRect rect = option.rect;
    editor->setGeometry(rect.x() + ITEM_MARGIN, rect.y(), ITEM_WIDTH, ITEM_HEIGHT);
}
