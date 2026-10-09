// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "listview.h"
#include "clipboardmodel.h"
#include "itemdelegate.h"

#include <QtTest>
#include <QDebug>
#include <QDrag>
#include <QSignalSpy>

namespace {
class DragTestListView : public ListView
{
public:
    using ListView::mousePressEvent;
    using ListView::mouseReleaseEvent;
};
}

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

TEST_F(TstListView, scrollToTest)
{
    // nothing happend
    list->scrollTo(QModelIndex());
}

TEST_F(TstListView, keyPressTest)
{
    list->show();
    QTest::qWait(1);

    QTest::keyPress(list, Qt::Key_Up);
    QTest::keyPress(list, Qt::Key_Down);

    QTest::mouseMove(list, QPoint(list->geometry().center()));

    // other key press
    QTest::keyPress(list, Qt::Key_Tab);
    QTest::keyPress(list, Qt::Key_Backtab);
    QTest::qWait(1);

    QTest::keyPress(list->viewport(), Qt::Key_Tab);
    QTest::keyPress(list->viewport(), Qt::Key_Backtab);
    QTest::qWait(1);
}

TEST_F(TstListView, uiTest)
{
    ClipboardModel *model = new ClipboardModel(list);
    ItemDelegate *delegate = new ItemDelegate(list);

    list->setItemDelegate(delegate);
    list->setModel(model);

    list->show();
    QTest::qWait(10);

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

    for (int i = 0; i < 10; ++i) {
        model->dataComing(textbuf);
        model->dataComing(imagebuf);
        model->dataComing(filebuf);
    }
    // 留出时间让listview绘制
    QTest::qWait(10);

    ASSERT_EQ(model->data().size(), 30);
    ASSERT_EQ(model->data().first()->urls().size(), 3);

    QSignalSpy spy(model, &ClipboardModel::dataChanged);

    model->destroy(model->data().first());
    QTest::qWait(AnimationTime + 20);
    ASSERT_EQ(spy.count(), 1);

    QVariant vaildVar = model->data(list->indexAt(QPoint(0, 0)), 0);
    ASSERT_TRUE(vaildVar.isValid());
    QVariant invalidVar = model->data(QModelIndex(), 0);
    ASSERT_FALSE(invalidVar.isValid());

    // 测试dataReborn信号发送
    QSignalSpy rebornSpy(model, &ClipboardModel::dataReborn);
    model->reborn(model->data().last());
    ASSERT_EQ(rebornSpy.count(), 1);

    model->reborn(model->data().first());
    ASSERT_EQ(rebornSpy.count(), 2);

    model->clear();
    ASSERT_EQ(spy.count(), 2);

    //    QThread::msleep(300 + 10);
    //    ASSERT_EQ(model->data().size(), 2);
}

TEST_F(TstListView, mousePressTest)
{
    list->show();
    QTest::qWait(1);

    list->setCurrentIndex(QModelIndex());
    QTest::mousePress(list, Qt::LeftButton, Qt::NoModifier);
}

TEST(TstListViewDrag, imageThenTextKeepsMimeDataSeparate)
{
    DragTestListView list;
    ClipboardModel model(nullptr);
    list.setModel(&model);
    list.setGridSize(QSize(200, 80));
    list.resize(240, 240);

    for (const auto &path : {":/qrc/text.buf", ":/qrc/image.buf"}) {
        QFile file(path);
        ASSERT_TRUE(file.open(QIODevice::ReadOnly));
        ASSERT_TRUE(QMetaObject::invokeMethod(&model, "dataComing", Qt::DirectConnection,
                                             Q_ARG(QByteArray, file.readAll())));
    }
    ASSERT_EQ(model.data().size(), 2);
    ASSERT_EQ(model.data().at(0)->type(), Image);
    ASSERT_EQ(model.data().at(1)->type(), Text);
    for (auto item : model.data())
        item->setParent(&model);

    list.show();
    QTest::qWait(1);

    auto pressRow = [&list, &model](int row) {
        list.setCurrentIndex(model.index(row, 0));
        const QPoint pos = list.visualRect(model.index(row, 0)).center();
        QMouseEvent press(QEvent::MouseButtonPress, pos, list.viewport()->mapToGlobal(pos),
                          Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        list.mousePressEvent(&press);
        return pos;
    };

    auto startDrag = [&list](const QPoint &pos) {
        // End the offscreen drag without delivering a release to the source view.
        QTimer cancelTimer;
        cancelTimer.setSingleShot(true);
        QObject::connect(&cancelTimer, &QTimer::timeout, &list, [] { QDrag::cancel(); });
        cancelTimer.start(10);
        QMouseEvent move(QEvent::MouseMove, pos, list.viewport()->mapToGlobal(pos),
                         Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
        list.mouseMoveEvent(&move);
    };

    startDrag(pressRow(0));
    QPointer<QDrag> imageDrag = list.findChild<QDrag *>();
    ASSERT_TRUE(imageDrag);
    ASSERT_TRUE(imageDrag->mimeData()->hasFormat("application/x-qt-image"));

    // X11 can retain a drag after exec() while the target is still reading it.
    // Keep the offscreen drag alive to reproduce that lifetime deterministically.
    QCoreApplication::removePostedEvents(imageDrag, QEvent::DeferredDelete);
    const QStringList imageFormats = imageDrag->mimeData()->formats();
    const QByteArray imageText = imageDrag->mimeData()->data("text/plain");

    const QPoint textPos = pressRow(1);
    ASSERT_EQ(list.currentIndex(), model.index(1, 0));
    ASSERT_EQ(imageDrag->mimeData()->formats(), imageFormats);
    ASSERT_EQ(imageDrag->mimeData()->data("text/plain"), imageText);

    startDrag(textPos);
    const auto drags = list.findChildren<QDrag *>();
    ASSERT_EQ(drags.size(), 2);
    QDrag *textDrag = drags.last();
    ASSERT_NE(textDrag->mimeData(), imageDrag->mimeData());
    EXPECT_EQ(textDrag->mimeData()->text(),
              QString::fromUtf8(model.data().at(1)->formatMap().value("text/plain")));
    EXPECT_FALSE(textDrag->mimeData()->hasFormat("application/x-qt-image"));
    EXPECT_FALSE(textDrag->mimeData()->hasFormat("image/png"));
    EXPECT_FALSE(textDrag->mimeData()->hasUrls());

    // A source-view release must not delete data now owned by a retained drag.
    QCoreApplication::removePostedEvents(textDrag, QEvent::DeferredDelete);
    QPointer<QMimeData> textMime = textDrag->mimeData();
    QMouseEvent release(QEvent::MouseButtonRelease, textPos, list.viewport()->mapToGlobal(textPos),
                        Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    list.mouseReleaseEvent(&release);
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    EXPECT_TRUE(textMime);
}
