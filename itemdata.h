#ifndef ITEMDATA_H
#define ITEMDATA_H

#include <QDateTime>
#include <QIcon>
#include <QMimeData>
#include <QObject>
#include <QPixmap>

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

    QIcon icon();          // 图标
    QString title();        // 类型名称
    QString subTitle();     // 字符数，像素信息，文件名称（多个文件显示XXX等X个文件）
    QString createTime();   // 复制时间
    QString contentText();  // 内容预览
    QPixmap contentImage(); // 缩略图

signals:

public slots:

private:
    DataType m_type;
    QIcon m_icon;
    QPixmap m_image;
    QString m_text;
    QDateTime m_createTime;
};

#endif // ITEMDATA_H
