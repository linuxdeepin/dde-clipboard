QT       += widgets core gui dbus gui-private testlib

TARGET = tst_dde-clipboard
TEMPLATE = app

PKGCONFIG += dtkwidget dtkgui dtkcore gio-qt dframeworkdbus
CONFIG += c++11 link_pkgconfig
CONFIG -= app_bundle

## 服务器编译时没有显示器，会卡住，等解决方案
#CONFIG += testcase no_testcase_installs

LIBS += -lgtest

DEFINES += QT_DEPRECATED_WARNINGS

include(../../dde-clipboard/src.pri)

SOURCES += \
    $$PWD/main.cpp \
    $$PWD/tst_mainwindow.cpp \
    $$PWD/tst_pixmaplabel.cpp \
    $$PWD/tst_clipboardmodel.cpp \
    $$PWD/tst_listview.cpp \
    $$PWD/tst_refreshtimer.cpp \
    $$PWD/tst_itemwidget.cpp \
    $$PWD/tst_iteminfo.cpp \
    $$PWD/tst_constants.cpp \
    $$PWD/tst_iconbutton.cpp \
    $$PWD/tst_itemdata.cpp

INCLUDEPATH += ../../dde-clipboard
INCLUDEPATH += ../../dde-clipboard/dbus

# gcov链接参数
QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0

RESOURCES += img.qrc
