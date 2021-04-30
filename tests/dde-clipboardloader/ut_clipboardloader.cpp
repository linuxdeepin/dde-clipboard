#include <gtest/gtest.h>
#include "clipboardloader.h"

#include <QApplication>
#include <QMimeData>
#include <QStyle>

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

    loader->cachePixmap(srcPix, info);
}

TEST_F(TstClipboardLoader, setImageData)
{
    QStyle *style = QApplication::style();
    const QPixmap &srcPix = style->standardPixmap(QStyle::SP_DialogYesButton);
    qApp->clipboard()->setPixmap(srcPix);
    ItemInfo info;

    const QMimeData *mime = qApp->clipboard()->mimeData();
    QMimeData *newData = new QMimeData;

    foreach (auto key, mime->formats()) {
        newData->setData(key, mime->data(key));
    }

    loader->setImageData(info, newData);

    delete newData;
    newData = nullptr;
}

