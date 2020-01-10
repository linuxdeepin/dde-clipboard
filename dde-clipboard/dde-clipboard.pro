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
PKGCONFIG += dtkwidget dtkgui gio-qt

DBUS_ADAPTORS += ./com.deepin.dde.Clipboard.xml

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        itemdata.cpp \
        clipboardmodel.cpp \
        dbusdisplay.cpp \
        itemwidget.cpp \
        pixmaplabel.cpp \
        dbusdock.cpp \
        dockrect.cpp \
    listview.cpp \
    itemdelegate.cpp \
    iconbutton.cpp \
    ClipboardLoaderInterface.cpp
HEADERS += \
        mainwindow.h \
        clipboardmodel.h \
        itemdata.h\
        dbusdisplay.h \
        constants.h \
        itemwidget.h \
        pixmaplabel.h \
        dbusdock.h \
        dockrect.h \
    listview.h \
    itemdelegate.h \
    iconbutton.h \
    refreshtimer.h \
    ClipboardLoaderInterface.h

DISTFILES += translate_generation.sh

TRANSLATIONS+= translations/$${TARGET}.ts\
                translations/$${TARGET}_zh_CN.ts

target.path = /usr/bin/
INSTALLS += target

translations.path = /usr/share/$${TARGET}/translations
translations.files = translations/*.qm
INSTALLS += translations

autostart.path = /etc/xdg/autostart
autostart.files = dde-clipboard.desktop
INSTALLS += autostart

services.path = /usr/share/dbus-1/services
services.files = com.deepin.dde.Clipboard.service
INSTALLS += services

CONFIG(release, debug|release) {
    !system(./translate_generation.sh): error("Failed to generate translation")
}
