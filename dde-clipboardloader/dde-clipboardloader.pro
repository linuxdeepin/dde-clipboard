QT += gui widgets dbus

TARGET = dde-clipboardloader
TEMPLATE = app

PKGCONFIG += dtkwidget
CONFIG += c++11 link_pkgconfig
CONFIG -= app_bundle

# 增加安全编译参数
QMAKE_LFLAGS += -z noexecstack -pie -z relro -z now
QMAKE_CFLAGS += -fstack-protector-all -D_FORTITY_SOURCE=1
QMAKE_CXXFLAGS += -fstack-protector-all -D_FORTITY_SOURCE=1

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(src.pri)

SOURCES += \
        $$PWD/main.cpp

INCLUDEPATH += ../dde-clipboard/

target.path = /usr/bin/
INSTALLS += target
