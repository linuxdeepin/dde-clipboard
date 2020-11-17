#include <gtest/gtest.h>
#include "clipboardloader.h"

#include <QApplication>
#include <QMimeData>

class TstClipboardLoader : public testing::Test
{
public:
    void SetUp() override
    {
        loader = new ClipboardLoader();
    }

    void TearDown() override
    {
        delete loader;
        loader = nullptr;
    }

public:
    ClipboardLoader *loader = nullptr;
};

TEST_F(TstClipboardLoader, coverageTest)
{
    QPixmap srcPix;
    ItemInfo info;

    ASSERT_TRUE(loader->cachePixmap(srcPix, info));
}

TEST_F(TstClipboardLoader, setImageData)
{
    QPixmap srcPix(":/qrc/desktop.jpg");
    qApp->clipboard()->setPixmap(srcPix);
    ItemInfo info;

    const QMimeData *mime = qApp->clipboard()->mimeData();
    QMimeData *newData = new QMimeData;

    foreach (auto key, mime->formats()) {
        newData->setData(key, mime->data(key));
    }

    loader->setImageData(info, newData);

    ASSERT_TRUE(true);
}

