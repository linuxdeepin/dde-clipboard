#include <gtest/gtest.h>
#include "listview.h"
#include "clipboardmodel.h"
#include "itemdelegate.h"

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
    ClipboardModel *model= nullptr;
    ItemDelegate *delegate = nullptr;
};

TEST_F(TstListView, coverageTest)
{
    ASSERT_NE(list, nullptr);
}
