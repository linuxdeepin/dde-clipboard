#include <gtest/gtest.h>

#include "itemdata.h"

#include <QDebug>

class TstItemData : public testing::Test
{
public:
    void SetUp() override
    {
//        ItemInfo info;
//        data = new ItemData(info);
    }

    void TearDown() override
    {
//        delete data;
//        data = nullptr;
    }

public:
//    ItemData *data = nullptr;
};

TEST_F(TstItemData, coverageTest)
{
//    data->setDataEnabled(false);
}
