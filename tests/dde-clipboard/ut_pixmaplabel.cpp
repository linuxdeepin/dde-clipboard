#include <gtest/gtest.h>
#include "pixmaplabel.h"

#include <QApplication>
#include <QTest>
#include <QScopedPointer>

class TstPixmapLabel : public testing::Test
{
};

TEST_F(TstPixmapLabel, pixmapTest)
{
    QStyle *style = QApplication::style();
    const QPixmap &pix = style->standardPixmap(QStyle::SP_DialogYesButton);
    QList<QPixmap> list;
    list << pix;

    // 只有一张图片
    QScopedPointer<PixmapLabel> pixmapLabel(new PixmapLabel(list));
    pixmapLabel->setPixmapList(list);
    pixmapLabel->show();
    QTest::qWait(1);
    pixmapLabel->setEnabled(false);
    QTest::qWait(1);

    // 两张图片
    list << pix;
    QScopedPointer<PixmapLabel> doublePixmapLabel(new PixmapLabel(list));
    pixmapLabel->show();
    QTest::qWait(1);
    pixmapLabel->setEnabled(false);
    QTest::qWait(1);

    // 三张图片
    list << pix;
    QScopedPointer<PixmapLabel> pixmapsLabel(new PixmapLabel(list));
    pixmapsLabel->setPixmapList(list);
    pixmapsLabel->show();
    QTest::qWait(1);
    pixmapsLabel->setEnabled(false);
    QTest::qWait(1);
    ASSERT_EQ(pixmapsLabel->pixmapList(), list);

    // 换一种构造函数
    QScopedPointer<PixmapLabel> pixmapsLabelTmp(new PixmapLabel(list));
    pixmapsLabelTmp->show();
    QTest::qWait(1);
    ASSERT_EQ(pixmapsLabelTmp->pixmapList(), list);
}

TEST_F(TstPixmapLabel, coverageTest)
{
    QScopedPointer<PixmapLabel> label(new PixmapLabel());

    QString text = "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < 10; ++i)
        text += text;

    // 直接设置文本
    label->setText(text);
    label->show();
    QTest::qWait(1);

    label->setAlignment(Qt::AlignRight);
    label->elideText(text.simplified(), QSize(100,80), QTextOption::WrapAnywhere, qApp->font(), Qt::ElideMiddle, 0);

    label->setAlignment(Qt::AlignHCenter);
    label->elideText(text.simplified(), QSize(100,80), QTextOption::WrapAnywhere, qApp->font(), Qt::ElideMiddle, 0);

    label->sizeHint();
    label->minimumSizeHint();
}
