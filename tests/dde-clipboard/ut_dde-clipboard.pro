QT       += widgets core gui dbus gui-private testlib

TARGET = tst_dde-clipboard
TEMPLATE = app

PKGCONFIG += dtkwidget dtkgui dtkcore gio-qt dframeworkdbus
CONFIG += c++11 link_pkgconfig
CONFIG -= app_bundle

CONFIG += testcase no_testcase_installs

LIBS += -lgtest

DEFINES += QT_DEPRECATED_WARNINGS

include(../../dde-clipboard/src.pri)

SOURCES += \
    $$PWD/ut_main.cpp \
    $$PWD/ut_mainwindow.cpp \
    $$PWD/ut_pixmaplabel.cpp \
    $$PWD/ut_listview.cpp \
    $$PWD/ut_refreshtimer.cpp \
    $$PWD/ut_itemwidget.cpp \
    $$PWD/ut_iteminfo.cpp \
    $$PWD/ut_constants.cpp \
    $$PWD/ut_iconbutton.cpp \
    $$PWD/ut_itemdata.cpp

INCLUDEPATH += ../../dde-clipboard
INCLUDEPATH += ../../dde-clipboard/dbus

# gcov链接参数
QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0

# 添加内存泄露检测
CONFIG(debug, debug|release) {
QMAKE_CXX += -g -fsanitize=address -O2
QMAKE_CXXFLAGS += -g -fsanitize=address -O2
QMAKE_LFLAGS += -g -fsanitize=address -O2
}

RESOURCES += \
    qrc.qrc
