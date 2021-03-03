#include <gtest/gtest.h>
#include "clipboardmodel.h"

#include "iconbutton.h"

#include <QtTest>
#include <QSignalSpy>
#include <QDebug>

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

TEST_F(TstIconButton, keyPressTest)
{
    QSignalSpy spy(btn, SIGNAL(clicked()));

    QTest::mouseClick(btn, Qt::LeftButton);
    ASSERT_TRUE(spy.count() == 1);

    QTest::mouseClick(btn, Qt::RightButton);
    ASSERT_TRUE(spy.count() == 1);

    QTest::mouseDClick(btn, Qt::RightButton);
    ASSERT_TRUE(spy.count() == 1);
}
