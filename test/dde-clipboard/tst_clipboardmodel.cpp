#include <gtest/gtest.h>
#include "clipboardmodel.h"

#include "listview.h"

class TstClipboardModel : public testing::Test
{
public:
    void SetUp() override
    {
        m_list = new ListView();
        model = new ClipboardModel(m_list);
    }

    void TearDown() override
    {
        delete model;
        model = nullptr;
    }

public:
    ClipboardModel *model = nullptr;
    ListView *m_list = nullptr;
};

TEST_F(TstClipboardModel, coverageTest)
{

}

TEST_F(TstClipboardModel, funcTest)
{
//    ItemInfo info;
//    model->destroy(new ItemData(info));
//    model->reborn(new ItemData(info));
//    model->clear();
}
