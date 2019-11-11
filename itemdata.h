#ifndef ITEMDATA_H
#define ITEMDATA_H

#include <QDateTime>
#include <QIcon>
#include <QMimeData>
#include <QObject>
#include <QPixmap>
#include <QUrl>

#include "constants.h"

class ItemData : public QObject
{
    Q_OBJECT
public:
    explicit ItemData(const QMimeData *mimeData);

    enum DataType {
        Text,
        Image,
        File
    };

    QIcon icon();                               // 图标
    QString title();                            // 类型名称
    QString subTitle();                         // 字符数，像素信息，文件名称（多个文件显示XXX等X个文件）
    const QList<QUrl> &urls();
    const QDateTime &createTime();              // 复制时间
    const QString &contentHtml();              // 富文本信息
    const QString &contentText();              // 内容预览
    const QPixmap &contentImage();             // 缩略图
    const DataType &type() {return m_type;}

private:
    DataType m_type;
    QList<QUrl> m_urls;
    QIcon m_icon;
    QPixmap m_image;
    QString m_html;
    QString m_text;
    QDateTime m_createTime;
};

#endif // ITEMDATA_H
