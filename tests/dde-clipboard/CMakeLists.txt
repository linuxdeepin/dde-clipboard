find_package(QT NAMES Qt5 REQUIRED COMPONENTS Core)
find_package(Qt5 COMPONENTS Gui Widgets DBus Test Concurrent REQUIRED)

find_package(Dtk COMPONENTS Widget Core REQUIRED)
find_package(DtkTools REQUIRED)
find_package(Threads REQUIRED)
pkg_check_modules(GIO REQUIRED IMPORTED_TARGET gio-qt)

file(GLOB_RECURSE Clipboard_SCRS
    "../../dde-clipboard/*.h"
    "../../dde-clipboard/*.cpp"
    "*.h"
    "*.cpp"
    "../../dbus/*.h"
    "../../dbus/*.cpp"
)
foreach(item ${Clipboard_SCRS})
    if(${item} MATCHES ".*/main.cpp")
        list(REMOVE_ITEM Clipboard_SCRS ${item})
    endif()
endforeach()

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/../../dbus
    ${CMAKE_CURRENT_LIST_DIR}/../../dde-clipboard
    ${CMAKE_CURRENT_LIST_DIR}/../../dde-clipboard/displaymanager/
    ${CMAKE_CURRENT_LIST_DIR}/../../dde-clipboard/dbus
)

qt_add_dbus_adaptor(Clipboard_SCRS
    ../../dbus/org.deepin.dde.Clipboard1.xml
    ../../dde-clipboard/mainwindow.h
    MainWindow
)

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/dbus
    ${CMAKE_CURRENT_LIST_DIR}/displaymanager
    ${CMAKE_CURRENT_LIST_DIR}/../dbus
)

dtk_add_dbus_interface(Clipboard_SCRS
    ../../dbus/org.deepin.dde.Display1.xml
    display_interface
)

dtk_add_dbus_interface(Clipboard_SCRS
    ../../dbus/org.deepin.dde.daemon.Dock1.xml
    dock_interface
)

dtk_add_dbus_interface(Clipboard_SCRS
    ../../dbus/org.deepin.dde.Display1.Monitor.xml
    monitor_interface
)

add_executable(ut-dde-clipboard
    ${Clipboard_SCRS}
    ./qrc.qrc
)

target_compile_options(ut-dde-clipboard PRIVATE -fprofile-arcs -ftest-coverage)

target_link_libraries(ut-dde-clipboard PRIVATE
    Dtk::Widget
    PkgConfig::GIO
    Qt5::Core
    Qt5::Widgets
    Qt5::DBus
    Qt5::Test
    Qt5::GuiPrivate
    GTest::gtest
    Threads::Threads
    -lgcov
    -lgtest
)

#gtest_discover_tests(ut-dde-clipboard WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
