#include "iteminfo.h"

#include <QUrl>

ItemInfo::ItemInfo()
{

}

QDebug operator<<(QDebug debug, const ItemInfo &info)
{
//    debug << QString("TestInfo(%1, %2, %3, %4, %5, %6, %7")
//          .arg(info.m_formatMap.size())
//          .arg(info.m_type)
//          .arg(info.m_urls.size())
//          .arg(info.m_enable)
//          .arg(info.m_text)
//          .arg(info.m_createTime.toString("yyyy-MM-dd hh:mm:ss:zzzz"))
//          .arg(info.m_iconDataList.size());

    return debug;
}

QDBusArgument &operator<<(QDBusArgument &arg, const ItemInfo &info)
{
    arg.beginStructure();

    foreach (auto buf, info.m_formatList) {
        arg << buf;
//        arg << 0x2;//间隔符
    }

//    QDBusVariant variant;
//    variant.setVariant(info.m_variantImage);

//    QStringList urls;
//    foreach (auto url, info.m_urls) {
//        urls.push_back(url.url());
//    }
//    QByteArray iconBuf;
//    QDataStream stream(&iconBuf, QIODevice::ReadOnly);
//    for (int i = 0; i < info.m_iconDataList.size(); ++i) {
//        FileIconData data = info.m_iconDataList[i];
//        stream << data.cornerIconList << data.fileIcon;
//    }

//    arg << info.m_formatMap << info.m_type << urls << variant
//        << info.m_enable << info.m_text << info.m_createTime << iconBuf;

    arg.endStructure();

    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, ItemInfo &info)
{
    arg.beginStructure();

    foreach (auto buf, info.m_formatList) {
        arg >> buf;
//        QByteArray temp;//去除间隔符
//        arg >> temp;
    }

//    QDBusVariant variant;
//    int type;
//    QStringList urls;
//    QByteArray iconBuf;

//    arg >> info.m_formatMap >> type >> urls >> variant
//        >> info.m_enable >> info.m_text >> info.m_createTime >> iconBuf;

//    foreach (QString url, urls) {
//        QUrl url_(url);
//        info.m_urls.push_back(url_);
//    }
//    info.m_type = static_cast<DataType>(type);
//    info.m_variantImage = variant.variant();

//    QDataStream stream(&iconBuf, QIODevice::ReadOnly);
//    for (int i = 0; i < info.m_urls.size(); ++i) {
//        FileIconData data;
//        stream >> data.cornerIconList >> data.fileIcon;
//        info.m_iconDataList.push_back(data);
//    }

    arg.endStructure();

    return arg;
}
