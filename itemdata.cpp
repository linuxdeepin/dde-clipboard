#include "itemdata.h"

#include <QDebug>
ItemData::ItemData(const QMimeData *mimeData)
{
    if (mimeData->hasImage()) {
        m_image = qvariant_cast<QPixmap>(mimeData->imageData());
        m_type = Image;
    } else if (mimeData->hasUrls()) {
        m_urls = mimeData->urls();
        m_type = File;
    } else {
        if (mimeData->hasHtml()) {
            m_text = mimeData->html();
        }
        if (mimeData->hasText()) {
            m_text = mimeData->text();
        }
        m_type = Text;
    }

    m_icon = QIcon::fromTheme("");
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
        return QString(tr("%1 characters")).arg(m_text.simplified().length());
    case File:
        return "";
    }

    return "";
}

const QList<QUrl> &ItemData::urls()
{
    return m_urls;
}

const QDateTime &ItemData::createTime()
{
    return m_createTime;
}

const QString &ItemData::contentHtml()
{
    return m_html;
}

const QString &ItemData::contentText()
{
    return m_text;
}

const QPixmap &ItemData::contentImage()
{
    return m_image;
}
