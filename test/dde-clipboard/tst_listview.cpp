#include <gtest/gtest.h>
#include "listview.h"
#include "clipboardmodel.h"
#include "itemdelegate.h"

#include <QtTest>
#include <QDebug>

class TstListView : public testing::Test
{
public:
    void SetUp() override
    {
        list = new ListView;
        model = new ClipboardModel(list);
        delegate = new ItemDelegate;

        list->setModel(model);
        list->setItemDelegate(delegate);
    }

    void TearDown() override
    {
        delete list;
        list = nullptr;

        delete model;
        model = nullptr;

        delete delegate;
        delegate = nullptr;
    }

public:
    ListView *list = nullptr;
    ClipboardModel *model = nullptr;
    ItemDelegate *delegate = nullptr;
};

TEST_F(TstListView, coverageTest)
{
    ASSERT_NE(list, nullptr);
}

TEST_F(TstListView, scrollToTest)
{
    list->scrollTo(QModelIndex());
}

TEST_F(TstListView, keyPressTest)
{
    list->show();

    QTest::keyPress(list, Qt::Key_Up);
    QTest::keyPress(list, Qt::Key_Down);

    QTest::mouseMove(list, QPoint(list->geometry().center()));

    // other key press
    QTest::keyPress(list, Qt::Key_A);
    QTest::keyPress(list, Qt::Key_Backtab);
}

TEST_F(TstListView, uiTest)
{
    ClipboardModel *model = new ClipboardModel(list);
    ItemDelegate *delegate = new ItemDelegate;

    list->setItemDelegate(delegate);

    QByteArray textbuf;
    QByteArray imagebuf;
    QByteArray filebuf;

    // 复制文本时产生的数据，用于测试
    QFile file1(":/qrc/text.buf");
    if (!file1.open(QIODevice::ReadOnly)) {
        qWarning() << "file open failed";
    } else {
        textbuf = file1.readAll();
    }
    file1.close();

    // 复制图片（非图片，图片文件属于文件类型）时产生的数据，用于测试
    QFile file2(":/qrc/image.buf");
    if (!file2.open(QIODevice::ReadOnly)) {
        qWarning() << "file open failed";
    } else {
        imagebuf = file2.readAll();
    }
    file2.close();

    // 复制文件时产生的数据，用于测试
    QFile file3(":/qrc/file.buf");
    if (!file3.open(QIODevice::ReadOnly)) {
        qWarning() << "file open failed";
    } else {
        filebuf = file3.readAll();
    }
    file3.close();

    model->dataComing(textbuf);
    model->dataComing(imagebuf);
    model->dataComing(filebuf);

    ASSERT_EQ(model->data().size(), 3);
    ASSERT_EQ(model->data().first()->urls().size(), 3);

    model->destroy(model->data().last());
//    QThread::msleep(300 + 10);
//    ASSERT_EQ(model->data().size(), 2);
}
