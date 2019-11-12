#include "itemdelegate.h"
#include "clipboardmodel.h"
#include "itemdata.h"
#include "itemwidget.h"

#include <QPointer>

DWIDGET_USE_NAMESPACE

ItemDelegate::ItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}

QWidget *ItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    QPointer<ItemData> data = index.data().value<QPointer<ItemData>>();
    ItemWidget *w = new ItemWidget(data, parent);
    return w;
}

QSize ItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option);
    Q_UNUSED(index);

    return QSize(ItemWidth, ItemHeight + ItemMargin);
}

void ItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    QRect rect = option.rect;
    editor->setGeometry(rect.x() + ItemMargin, rect.y(), ItemWidth, ItemHeight);
}
