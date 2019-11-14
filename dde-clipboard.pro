#-------------------------------------------------
#
# Project created by QtCreator 2019-11-01T13:38:32
#
#-------------------------------------------------

QT       += core gui dbus
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = dde-clipboard
TEMPLATE = app
CONFIG += c++11 link_pkgconfig
PKGCONFIG += dtkwidget dtkgui

SOURCES += \
        main.cpp \
        mainwindow.cpp \
        itemdata.cpp \
        clipboardmodel.cpp \
        dbusdisplay.cpp \
        itemdelegate.cpp \
        itemwidget.cpp \
        pixmaplabel.cpp

HEADERS += \
        mainwindow.h \
        clipboardmodel.h \
        itemdata.h\
        dbusdisplay.h \
        itemdelegate.h \
        constants.h \
        itemwidget.h \
        pixmaplabel.h

DISTFILES += translate_generation.sh

TRANSLATIONS+= translations/$${TARGET}.ts\
                translations/$${TARGET}_zh_CN.ts

target.path = /usr/bin/
INSTALLS += target

CONFIG(release, debug|release) {
    !system(./translate_generation.sh): error("Failed to generate translation")
}
