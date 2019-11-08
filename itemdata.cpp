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

    m_createTime = QDateTime::currentDateTime();
}

QIcon ItemData::icon()
{
    return m_icon;
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

    return "";
}

QString ItemData::subTitle()
{
    switch (m_type) {
    case Image:
        return "";
    case Text:
        return QString(tr("%1 characters")).arg(m_text.length());
    case File:
        return "";
    }

    return "";
}

QString ItemData::createTime()
{
    return "";
}

QString ItemData::contentText()
{
    return m_text;
}

QPixmap ItemData::contentImage()
{
    return m_image;
}
