QT       += widgets core gui dbus gui-private testlib

TARGET = tst_dde-clipboardloader
TEMPLATE = app

PKGCONFIG += dtkwidget dtkgui gio-qt dframeworkdbus
CONFIG += c++11 link_pkgconfig
CONFIG -= app_bundle

CONFIG += testcase no_testcase_installs

LIBS += -lgtest

DEFINES += QT_DEPRECATED_WARNINGS

include(../../dde-clipboardloader/src.pri)

SOURCES += \
    $$PWD/ut_main.cpp \
    $$PWD/ut_clipboardloader.cpp

INCLUDEPATH += ../../dde-clipboardloader

# gcov链接参数
QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0

# 添加内存泄露检测
CONFIG(debug, debug|release) {
QMAKE_CXX += -g -fsanitize=address -O2
QMAKE_CXXFLAGS += -g -fsanitize=address -O2
QMAKE_LFLAGS += -g -fsanitize=address -O2
}
