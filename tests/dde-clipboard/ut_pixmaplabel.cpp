#include <gtest/gtest.h>
#include "pixmaplabel.h"

#include <QApplication>

class TstPixmapLabel : public testing::Test
{
public:
    void SetUp() override
    {
        QStyle *style = QApplication::style();
        const QPixmap &pix = style->standardPixmap(QStyle::SP_DialogYesButton);
        QList<QPixmap> list;
        list << pix;
        list << pix;

        label = new PixmapLabel();
        label->setPixmapList(list);

        label2 = new PixmapLabel(list);
    }

    void TearDown() override
    {
        delete label;
        label = nullptr;

        delete label2;
        label2 = nullptr;
    }

public:
    PixmapLabel *label = nullptr;
    PixmapLabel *label2 = nullptr;
};

TEST_F(TstPixmapLabel, coverageTest)
{
    QString text = "abcdefghijklmnopqrstuvwxyz";

    label->setText(text);

    label->setAlignment(Qt::AlignRight);
    label->elideText(text.simplified(), QSize(100,80), QTextOption::WrapAnywhere, qApp->font(), Qt::ElideMiddle, 0);

    label->setAlignment(Qt::AlignHCenter);
    label->elideText(text.simplified(), QSize(100,80), QTextOption::WrapAnywhere, qApp->font(), Qt::ElideMiddle, 0);

    label->sizeHint();
    label->minimumSizeHint();
}
