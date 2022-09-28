// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "waylandcopyclient.h"

#include <QApplication>
#include <QMimeData>
#include <QStyle>
#include <QSignalSpy>
#include <QClipboard>
#include <QTest>

#ifdef USE_DEEPIN_KF5_WAYLAND
class TstWaylandCopyClient : public testing::Test
{
public:
    void SetUp() override
    {
        client = new WaylandCopyClient;
        client->init();
    }

    void TearDown() override
    {
        delete client;
    }

public:
    WaylandCopyClient *client;
};

TEST_F(TstWaylandCopyClient, copyTest)
{
    if (qEnvironmentVariable("XDG_SESSION_TYPE").contains("wayland")) {
        ASSERT_TRUE(client->mimeData()->formats().isEmpty());
        qApp->clipboard()->setText("test");

        // 从复制数据到检测到数据被复制，会有时间差，采用非阻塞等待的方式，延时250ms
        QTest::qWait(250);

        ASSERT_TRUE(client->mimeData()->hasText());
    }
}

TEST_F(TstWaylandCopyClient, mimeData)
{
    if (qEnvironmentVariable("XDG_SESSION_TYPE").contains("wayland")) {
        ASSERT_TRUE(client->mimeData());
    }
}
#endif
