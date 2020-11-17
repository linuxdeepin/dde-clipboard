#include <gtest/gtest.h>

#include "itemwidget.h"

class TstItemWidget : public testing::Test
{
public:
    void SetUp() override
    {
//        ItemInfo info;
//        widget = new ItemWidget(new ItemData(info));
    }

    void TearDown() override
    {
//        delete widget;
//        widget = nullptr;
    }

public:
//    ItemWidget *widget = nullptr;
};

TEST_F(TstItemWidget, coverageTest)
{
//    QPixmap testPix(":/img/testPix.png");

//    FileIconData data;
//    widget->setThumnail(testPix);
//    widget->setThumnail(data);
//    widget->setFileIcon(testPix);
//    widget->setFileIcon(data);

//    QList<QPixmap> list;
//    list << testPix;
//    list << testPix;
//    widget->setFileIcons(list);
}

TEST_F(TstItemWidget, propertyTest)
{
//    qDebug() << widget->opacity();
//    QString text = "abcdefghijklmnopqrstuvwxyz";
//    widget->setText(text, QString::number(text.length()));
//    widget->setAlpha(120);
//    qDebug() << widget->hoverAlpha() << widget->unHoverAlpha();
//    widget->setOpacity(0.5);
}

TEST_F(TstItemWidget, method_getCornerGeometryList_Test)
{
//    QRectF rectF(0,0,100,100);
//    QSizeF sizeF(50,50);
//    widget->getCornerGeometryList(rectF, sizeF);
}

TEST_F(TstItemWidget, method_getIconPixmap_Test)
{
//    ItemWidget::getIconPixmap(QIcon::fromTheme("close"), QSize(24, 24), 1.25, QIcon::Normal, QIcon::On);
}

TEST_F(TstItemWidget, method_GetFileIcon_Test)
{
//    ItemWidget::GetFileIcon("123.png");//不存在的图片
}
