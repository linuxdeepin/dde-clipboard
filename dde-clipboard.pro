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
    itemwidget.cpp

HEADERS += \
        mainwindow.h \
        clipboardmodel.h \
        itemdata.h\
    dbusdisplay.h \
    itemdelegate.h \
    constants.h \
    itemwidget.h

target.path = /usr/bin/
INSTALLS += target
