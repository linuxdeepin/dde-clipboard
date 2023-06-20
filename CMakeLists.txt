cmake_minimum_required(VERSION 3.16)

set(BIN_NAME dde-clipboard)

project(${BIN_NAME})

#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=address -O0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address -O0")
endif()

if (DEFINED ENABLE_MIEEE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
endif()

include(GNUInstallDirs)
# Install settings
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

if (NOT (${CMAKE_BUILD_TYPE} MATCHES "Debug"))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")

    # generate qm
    execute_process(COMMAND bash "translate_generation.sh"
                    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif ()

# Find the library
find_package(PkgConfig REQUIRED)
find_package(Dtk COMPONENTS Widget Core REQUIRED)
find_package(Qt5 COMPONENTS Core Gui Widgets DBus Test REQUIRED)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
find_package(DWayland REQUIRED)
#find_package(KF5Wayland REQUIRED)

pkg_check_modules(GIO REQUIRED IMPORTED_TARGET gio-qt)

try_compile(USE_DEEPIN_KF5_WAYLAND
              ${CMAKE_CURRENT_BINARY_DIR}/try_compile
            SOURCES
              ${CMAKE_CURRENT_SOURCE_DIR}/tests/kf5_wayland_test.cc
            LINK_LIBRARIES
              DWaylandClient
)

message(STATUS "USE_DEEPIN_KF5_WAYLAND ${USE_DEEPIN_KF5_WAYLAND}")

if(USE_DEEPIN_KF5_WAYLAND)
  add_definitions(-DUSE_DEEPIN_KF5_WAYLAND)
endif()

find_package(DtkTools REQUIRED)
function(generation_dbus_adaptor xml class_name class_file option)
    execute_process(COMMAND qdbusxml2cpp ${option} -a ${class_file} -c ${class_name} ${xml}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endfunction(generation_dbus_adaptor)

set(DBUS_INTERFACE ${PROJECT_SOURCE_DIR}/dbus/interface)
function(generation_dbus_interface xml class_name class_file)
    execute_process(COMMAND ${DTK_XML2CPP} -c ${class_name} -p ${class_file} ${xml}
        WORKING_DIRECTORY ${DBUS_INTERFACE})
endfunction(generation_dbus_interface)

include_directories(
    dde-clipboard
    dde-clipboard/dbus
    dde-clipboard/displaymanager
    dde-clipboardloader
)

#----------------------------dde-clipboard------------------------------
generation_dbus_adaptor(
    ${CMAKE_SOURCE_DIR}/dde-clipboard/org.deepin.dde.Clipboard1.xml
    ClipboardAdaptor
    ${CMAKE_CURRENT_BINARY_DIR}/clipboard_adaptor
    -N
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/dbus/org.deepin.dde.Display1.Monitor.xml
    Monitor
    ${DBUS_INTERFACE}/monitor_interface
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/dbus/org.deepin.dde.Display1.xml
    Display1
    ${DBUS_INTERFACE}/display_interface
)

generation_dbus_interface(
    ${CMAKE_SOURCE_DIR}/dbus/org.deepin.dde.daemon.Dock1.xml
    Dock1
    ${DBUS_INTERFACE}/dock_interface
)

include_directories(
    ${PROJECT_SOURCE_DIR}/dbus/interface
    ${PROJECT_SOURCE_DIR}/dbus/types
    ${PROJECT_SOURCE_DIR}/dbus
)

aux_source_directory(dbus/types DBUS_TYPES)
aux_source_directory(dbus/interface DBUS_INTERFACES)

set(Clipboard_DBUS_SCRS
    ${CMAKE_CURRENT_BINARY_DIR}/clipboard_adaptor.h
    ${CMAKE_CURRENT_BINARY_DIR}/clipboard_adaptor.cpp
)

file(GLOB_RECURSE Clipboard_SCRS
    "dde-clipboard/*.h"
    "dde-clipboard/*.cpp"
)
list(REMOVE_ITEM Clipboard_SCRS "${CMAKE_SOURCE_DIR}/dde-clipboard/main.cpp")

add_executable(${BIN_NAME}
    ${Clipboard_SCRS}
    ${Clipboard_DBUS_SCRS}
    dde-clipboard/main.cpp
    ${DBUS_INTERFACES}
    ${DBUS_TYPES}
)

target_link_libraries(${BIN_NAME} PRIVATE
    Dtk::Widget
    Dtk::Core
    PkgConfig::GIO
    Qt5::Core
    Qt5::Widgets
    Qt5::GuiPrivate
    Qt5::Gui
    Qt5::DBus
)

## qm files
file(GLOB QM_FILES "translations/*.qm")
install(FILES ${QM_FILES} DESTINATION share/${BIN_NAME}/translations)

## desktop file
file(GLOB DESKTOP_FILE "misc/dde-clipboard.desktop")
install(FILES ${DESKTOP_FILE} DESTINATION share/applications)

## autostart
set(AUTOSTART_FILE ${DESKTOP_FILE})
install(FILES ${AUTOSTART_FILE} DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/xdg/autostart)

## dbus service
file(GLOB SERVICE_FILE "misc/org.deepin.dde.Clipboard1.service")
install(FILES ${SERVICE_FILE} DESTINATION share/dbus-1/services)

## bin
install(TARGETS ${BIN_NAME} DESTINATION bin)

#----------------------------dde-clipboard-daemon------------------------------
set(BIN_NAME dde-clipboard-daemon)

file(GLOB_RECURSE dde-clipboard-daemon_SCRS
        "dde-clipboard-daemon/*.h"
        "dde-clipboard-daemon/*.cpp"
)

add_executable(${BIN_NAME}
    ${dde-clipboard-daemon_SCRS}
)

target_link_libraries(${BIN_NAME} PRIVATE
  Qt5::Gui
  Qt5::DBus
  Qt5::Widgets
  Dtk::Core
  DWaylandClient
)

if (NOT DEFINED SYSTEMD_USER_UNIT_DIR)
    pkg_get_variable(SYSTEMD_USER_UNIT_DIR systemd systemduserunitdir)
endif()

install(TARGETS ${BIN_NAME} DESTINATION bin)
install(FILES misc/dde-clipboard-daemon.service
    DESTINATION ${SYSTEMD_USER_UNIT_DIR}
)
install(FILES misc/dde-clipboard-daemon.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications/
)
install(FILES misc/dde-clipboard-daemon.desktop
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/xdg/autostart/
)

#----------------------------ut-dde-clipboard------------------------------
set(UT_BIN_NAME ut-dde-clipboard)

file(GLOB_RECURSE ut_Clipboard_SCRS
    "tests/dde-clipboard/*.h"
    "tests/dde-clipboard/*.cpp"
    "tests/dde-clipboard/*.qrc"
)

add_executable(${UT_BIN_NAME}
    ${Clipboard_SCRS}
    ${Clipboard_DBUS_SCRS}
    ${ut_Clipboard_SCRS}
    ${DBUS_INTERFACES}
    ${DBUS_TYPES}
)
# 用于测试覆盖率的编译条件
target_compile_options(${UT_BIN_NAME} PRIVATE -fprofile-arcs -ftest-coverage)

target_link_libraries(${UT_BIN_NAME} PRIVATE
    Dtk::Widget
    PkgConfig::GIO
    Qt5::Core
    Qt5::Widgets
    Qt5::DBus
    Qt5::Test
    Qt5::GuiPrivate
    -lpthread
    -lgcov
    -lgtest
)
