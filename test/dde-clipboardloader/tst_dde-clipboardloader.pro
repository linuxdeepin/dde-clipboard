QT       += widgets core gui dbus gui-private

TARGET = tst_dde-clipboardloader
TEMPLATE = app

## 服务器编译时没有显示器，会卡住，等解决方案
#CONFIG += testcase no_testcase_installs

PKGCONFIG += dtkwidget dtkgui gio-qt dframeworkdbus
CONFIG += c++11 link_pkgconfig
CONFIG -= app_bundle

LIBS += -lgtest

DEFINES += QT_DEPRECATED_WARNINGS

include(../../dde-clipboardloader/src.pri)

SOURCES += \
    $$PWD/main.cpp \
    $$PWD/tst_clipboardloader.cpp

INCLUDEPATH += ../../dde-clipboardloader

# gcov链接参数
QMAKE_CXXFLAGS += -g -Wall -fprofile-arcs -ftest-coverage -O0
QMAKE_LFLAGS += -g -Wall -fprofile-arcs -ftest-coverage  -O0

RESOURCES += \
    $$PWD/qrc.qrc
