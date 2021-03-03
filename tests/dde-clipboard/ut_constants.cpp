#include <gtest/gtest.h>
#include "clipboardmodel.h"

#include "constants.h"

class TstConstants : public testing::Test
{
public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_F(TstConstants, coverageTest)
{
    Globals::GetScale(QSize(1000,1000), 200, 300);
    Globals::GetScale(QSize(200,300), 1000, 1000);

    QPixmap pix;
    QPixmap testPix(":/qrc/testPix.png");
    Globals::pixmapScaled(pix);
    Globals::GetRoundPixmap(pix, QColor(Qt::red));

    Globals::pixmapScaled(testPix);
    Globals::GetRoundPixmap(testPix, QColor(Qt::red));
}
