#include <gtest/gtest.h>
#include "mainwindow.h"

#include <QTest>
#include <QLabel>

class TstMainWindow : public testing::Test
{
public:
    void SetUp() override
    {
        window = new MainWindow();
    }

    void TearDown() override
    {
        delete window;
        window = nullptr;
    }

public:
    MainWindow *window = nullptr;
};

TEST_F(TstMainWindow, coverage_Test)
{
    ASSERT_NE(window, nullptr);
}

TEST_F(TstMainWindow, showAni_Test)
{
    window->showAni();

    QThread::msleep(450);

    ASSERT_EQ(window->isVisible(), true);
}

TEST_F(TstMainWindow, hideAni_Test)
{
    window->hideAni();

    QThread::msleep(450);

    ASSERT_EQ(window->isVisible(), false);
}

TEST_F(TstMainWindow, Toggle_Test)
{
    bool visible = window->isVisible();

    window->Toggle();
    QThread::msleep(450);
    ASSERT_TRUE(!visible);
}

TEST_F(TstMainWindow, startLoader_Test)
{
    window->startLoader();

    const QByteArray processName = "dde-clipboardloader";

    QProcess ps;
    QStringList args;
    args<<"-c";
    args<<QString("ps -ef | grep %1").arg(QString::fromLatin1(processName));
    ps.start("sh",args);
    ps.waitForReadyRead();
    QByteArray buf = ps.readAllStandardOutput();
    ps.close();

    EXPECT_TRUE(buf.contains("/usr/bin/" + processName));
}
