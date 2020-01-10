#include <QDebug>
#include <QDBusArgument>
#include <QDateTime>
#include <QUrl>

#include "constants.h"
///*!
// * \~chinese \brief 数据类型
// */
//enum DataType {
//    Unknown,
//    Text,
//    Image,
//    File
//};

struct ItemInfo {
public:
    ItemInfo();

    friend QDebug operator<<(QDebug debug, const ItemInfo &rect);
    friend const QDBusArgument &operator>>(const QDBusArgument &arg, ItemInfo &rect);
    friend QDBusArgument &operator<<(QDBusArgument &arg, const ItemInfo &rect);

    QByteArrayList m_formatList;

//    QMap<QString, QByteArray> m_formatMap;
//    DataType m_type = Unknown;
//    QList<QUrl> m_urls;
//    QVariant m_variantImage = QVariant();
//    bool m_enable;
//    QString m_text;
//    QDateTime m_createTime;
//    QList<FileIconData> m_iconDataList;

    //other
//    bool m_hasImageData = false;
};

Q_DECLARE_METATYPE(ItemInfo)

