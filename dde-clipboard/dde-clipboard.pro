#-------------------------------------------------
#
# Project created by QtCreator 2019-11-01T13:38:32
#
#-------------------------------------------------

QT       += core gui dbus gui-private
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = dde-clipboard
TEMPLATE = app
CONFIG += c++11 link_pkgconfig
PKGCONFIG += dtkwidget dtkgui gio-qt dframeworkdbus

# 增加安全编译参数
QMAKE_LFLAGS += -z noexecstack -pie -z relro -z now
QMAKE_CFLAGS += -fstack-protector-all -D_FORTITY_SOURCE=1
QMAKE_CXXFLAGS += -fstack-protector-all -D_FORTITY_SOURCE=1

DBUS_ADAPTORS += ./com.deepin.dde.Clipboard.xml

include(src.pri)

SOURCES += \
        $$PWD/main.cpp

DISTFILES += ../translate_generation.sh

TRANSLATIONS+= ../translations/$${TARGET}.ts\
                ../translations/$${TARGET}_zh_CN.ts

target.path = /usr/bin/
INSTALLS += target

## desktop file
desktop.path = /usr/share/applications
desktop.files = dde-clipboard.desktop
INSTALLS += desktop

## translation
translations.path = /usr/share/$${TARGET}/translations
translations.files = ../translations/*.qm
INSTALLS += translations

## autostart
autostart.path = /etc/xdg/autostart
autostart.files = dde-clipboard.desktop
INSTALLS += autostart

## dbus service
services.path = /usr/share/dbus-1/services
services.files = com.deepin.dde.Clipboard.service
INSTALLS += services
