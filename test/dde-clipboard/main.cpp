#include <gtest/gtest.h>

#include <QApplication>
#include <QDebug>

#include <DLog>

DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    QApplication app(argc,argv);

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    qDebug() << "start dde-clipboard test cases ..............";
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    qDebug() << "end dde-clipboard test cases ..............";
    return ret;
}
