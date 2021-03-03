#include <gtest/gtest.h>

#include "iteminfo.h"

#include <QDebug>

class TstItemInfo : public testing::Test
{
public:
    void SetUp() override
    {
        info = new ItemInfo();
    }

    void TearDown() override
    {
       delete info;
       info = nullptr;
    }

public:
    ItemInfo *info = nullptr;
};

TEST_F(TstItemInfo, coverageTest)
{
    
}
