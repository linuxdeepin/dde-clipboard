#include <gtest/gtest.h>

#include "itemwidget.h"
#include "itemdata.h"

#include <QFile>
#include <QApplication>

class TstItemWidget : public testing::Test
{
public:
    void SetUp() override
    {
        QByteArray buf;

        // 复制图片（非图片，图片文件属于文件类型）时产生的数据，用于测试
        QFile file(":/qrc/image.buf");
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "file open failed";
            file.close();
        } else {
            buf = file.readAll();
        }

        widget = new ItemWidget(new ItemData(buf));
    }

    void TearDown() override
    {
        delete widget;
        widget = nullptr;
    }

public:
    ItemWidget *widget = nullptr;
};

TEST_F(TstItemWidget, coverageTest)
{
    QStyle *style = QApplication::style();
    const QPixmap &testPix = style->standardPixmap(QStyle::SP_DialogYesButton);

    FileIconData data;
    widget->setThumnail(testPix);
    widget->setThumnail(data);
    widget->setFileIcon(testPix);
    widget->setFileIcon(data);

    QList<QPixmap> list;
    list << testPix;
    list << testPix;
    widget->setFileIcons(list);
}

TEST_F(TstItemWidget, textTest)
{
    QByteArray buf;

    // 复制文本时产生的数据，用于测试
    QFile file(":/qrc/text.buf");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "file open failed";
        file.close();
    } else {
        buf = file.readAll();
    }

    delete widget;
    widget = nullptr;
    widget = new ItemWidget(new ItemData(buf));

    ASSERT_TRUE(widget->text().contains("1234567890abcdefg"));
}

TEST_F(TstItemWidget, fileTest)
{
    QByteArray buf;

    // 复制文件时产生的数据，用于测试
    QFile file(":/qrc/file.buf");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "file open failed";
        file.close();
    } else {
        buf = file.readAll();
    }

    delete widget;
    widget = nullptr;
    widget = new ItemWidget(new ItemData(buf));

    ASSERT_EQ(widget->itemData()->urls().first().toLocalFile(), "/home/diesel/Desktop/截图录屏_deepin-terminal_20201114221419.png");
}

TEST_F(TstItemWidget, propertyTest)
{
    QString text = "abcdefghijklmnopqrstuvwxyz";
    widget->setText(text, QString::number(text.length()));
    widget->setAlpha(120);
    widget->setOpacity(0.5);
}

TEST_F(TstItemWidget, method_getCornerGeometryList_Test)
{
    QRectF rectF(0, 0, 100, 100);
    QSizeF sizeF(50, 50);

    ASSERT_EQ(widget->getCornerGeometryList(rectF, sizeF).size(), 4);
}

TEST_F(TstItemWidget, method_getIconPixmap_Test)
{
    ASSERT_TRUE(ItemWidget::getIconPixmap(QIcon::fromTheme("close"), QSize(24, 24), 1.25, QIcon::Normal, QIcon::On).isNull());
}

TEST_F(TstItemWidget, method_GetFileIcon_Test)
{
    ASSERT_TRUE(ItemWidget::GetFileIcon("123.png").isNull());//不存在的图片
}
