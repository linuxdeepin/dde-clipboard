#include <gtest/gtest.h>

#include "refreshtimer.h"

class TstRefreshTimer : public testing::Test
{
public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

TEST_F(TstRefreshTimer, coverageTest)
{
    RefreshTimer::instance();
}
