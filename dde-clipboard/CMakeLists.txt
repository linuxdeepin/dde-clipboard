set(BIN_NAME dde-clipboard)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui Widgets DBus Test Concurrent REQUIRED)

find_package(Dtk6 COMPONENTS Widget Core REQUIRED)
find_package(Dtk6Tools)

file(GLOB_RECURSE Clipboard_SCRS
    "*.h"
    "*.cpp"
    "../dbus/*.h"
    "../dbus/*.cpp"
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
    Dtk6::Widget
    Dtk6::Core
    #PkgConfig::GIO
    Qt::Core
    Qt::Widgets
    Qt::GuiPrivate
    Qt::Gui
    Qt::DBus
)

install(TARGETS dde-clipboard DESTINATION ${CMAKE_INSTALL_BINDIR})
