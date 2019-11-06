#include "itemdata.h"

ItemData::ItemData(const QMimeData *mimeData)
{
    if (mimeData->hasImage()) {
        m_image = qvariant_cast<QPixmap>(mimeData->imageData());
        m_icon = QIcon::fromTheme("");
        m_type = Image;
    } else {
        if (mimeData->hasHtml()) {
            m_text = mimeData->html();
        } else if (mimeData->hasText()) {
            m_text = mimeData->text();
        }
        m_icon = QIcon::fromTheme("");
    }
}

QIcon *ItemData::icon()
{

}

QString ItemData::title()
{
    switch (m_type) {
    case Image:
        return tr("Picture");
    case Text:
        return tr("Text");
    case File:
        return tr("File");
    }
}

QString ItemData::subTitle()
{

}

QString ItemData::createTime()
{

}

QString ItemData::contentText()
{

}

QString ItemData::contentImage()
{

}
