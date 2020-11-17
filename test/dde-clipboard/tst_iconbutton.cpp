#include <gtest/gtest.h>
#include "clipboardmodel.h"

#include "iconbutton.h"

class TstIconButton : public testing::Test
{
public:
    void SetUp() override
    {
        btn = new IconButton();
    }

    void TearDown() override
    {
        delete btn;
        btn = nullptr;
    }

public:
    IconButton *btn = nullptr;
};

TEST_F(TstIconButton, coverageTest)
{
    btn->setText("abcdefghijklmnopqrstuvwxyz");
    btn->setFocusState(false);
    btn->setBackOpacity(-100);
    btn->setRadius(10000);
}
