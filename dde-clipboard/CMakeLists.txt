set(BIN_NAME dde-clipboard)

find_package(QT NAMES Qt5 REQUIRED COMPONENTS Core)
find_package(Qt5 COMPONENTS Gui Widgets DBus Test Concurrent REQUIRED)

find_package(Dtk COMPONENTS Widget Core REQUIRED)
find_package(DtkTools)

pkg_check_modules(GIO REQUIRED IMPORTED_TARGET gio-qt)

include(clipboardsrc.cmake)

set(Clipboard_SCRS
    ${Clipboard_SCRS_PRE}
    main.cpp
)

qt_add_dbus_adaptor(Clipboard_SCRS
    ../dbus/org.deepin.dde.Clipboard1.xml
    mainwindow.h
    MainWindow
)

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/dbus
    ${CMAKE_CURRENT_LIST_DIR}/displaymanager
    ${CMAKE_CURRENT_LIST_DIR}/../dbus
)

dtk_add_dbus_interface(Clipboard_SCRS
    ../dbus/org.deepin.dde.Display1.xml
    display_interface
)

dtk_add_dbus_interface(Clipboard_SCRS
    ../dbus/org.deepin.dde.daemon.Dock1.xml
    dock_interface
)

dtk_add_dbus_interface(Clipboard_SCRS
    ../dbus/org.deepin.dde.Display1.Monitor.xml
    monitor_interface
)

add_executable(dde-clipboard
    ${Clipboard_SCRS}
)

target_link_libraries(dde-clipboard PRIVATE
    Dtk::Widget
    Dtk::Core
    PkgConfig::GIO
    Qt5::Core
    Qt5::Widgets
    Qt5::GuiPrivate
    Qt5::Gui
    Qt5::DBus
)

install(TARGETS dde-clipboard DESTINATION ${CMAKE_INSTALL_BINDIR})
