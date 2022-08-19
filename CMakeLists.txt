cmake_minimum_required(VERSION 3.13)

set(BIN_NAME dde-clipboard)

project(${BIN_NAME})

#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "-g -Wall")

# 增加安全编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fstack-protector-all")
set(CMAKE_EXE_LINKER_FLAGS  "-z relro -z now -z noexecstack -pie")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -fsanitize=address -O0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -fsanitize=address -O0")
endif()

if (DEFINED ENABLE_MIEEE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mieee")
endif()

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
find_package(DtkWidget REQUIRED)
find_package(Qt5 COMPONENTS Core Widgets DBus Test REQUIRED)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
find_package(DWayland REQUIRED)

pkg_check_modules(DFrameworkDBus REQUIRED dframeworkdbus)
pkg_check_modules(GIO REQUIRED gio-qt)

function(generation_dbus_adaptor xml class_name class_file option)
    execute_process(COMMAND qdbusxml2cpp ${option} -a ${class_file} -c ${class_name} ${xml}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endfunction(generation_dbus_adaptor)

include_directories(
    dde-clipboard
    dde-clipboard/dbus
    dde-clipboard/displaymanager
    dde-clipboardloader
)

#----------------------------dde-clipboard------------------------------
generation_dbus_adaptor(
    ${CMAKE_SOURCE_DIR}/dde-clipboard/com.deepin.dde.Clipboard.xml
    ClipboardAdaptor
    ${CMAKE_CURRENT_BINARY_DIR}/clipboard_adaptor
    -N
)

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
)
target_include_directories(${BIN_NAME} PUBLIC
    ${DtkWidget_INCLUDE_DIRS}
    ${GIO_INCLUDE_DIRS}
    ${DFrameworkDBus_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    Qt5::Core
    Qt5::Widgets
    Qt5::DBus
)

target_link_libraries(${BIN_NAME} PRIVATE
    ${DtkWidget_LIBRARIES}
    ${GIO_LIBRARIES}
    ${DFrameworkDBus_LIBRARIES}
    Qt5::Core
    Qt5::Widgets
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
install(FILES ${AUTOSTART_FILE} DESTINATION /etc/xdg/autostart)

## dbus service
file(GLOB SERVICE_FILE "misc/com.deepin.dde.Clipboard.service")
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

target_include_directories(${BIN_NAME} PUBLIC
  ${Qt5Widget_INCLUDE_DIRS}
  ${Qt5Core_INCLUDE_DIRS}
  ${DtkCore_INCLUDE_DIRS}
  )

target_link_libraries(${BIN_NAME} PRIVATE
  ${Qt5DBus_LIBRARIES}
  ${Qt5Core_LIBRARIES}
  ${Qt5Widgets_LIBRARIES}
  ${DtkCore_LIBRARIES}
  DWaylandClient
)

install(TARGETS ${BIN_NAME} DESTINATION bin)
install(FILES misc/dde-clipboard-daemon.service
    DESTINATION /lib/systemd/user
)
install(FILES misc/dde-clipboard-daemon.desktop
    DESTINATION /usr/share/applications/
)
install(FILES misc/dde-clipboard-daemon.desktop
    DESTINATION /etc/xdg/autostart/
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
)
# 用于测试覆盖率的编译条件
target_compile_options(${UT_BIN_NAME} PRIVATE -fprofile-arcs -ftest-coverage)

target_include_directories(${UT_BIN_NAME} PUBLIC
    ${DtkWidget_INCLUDE_DIRS}
    ${GIO_INCLUDE_DIRS}
    ${DFrameworkDBus_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    Qt5::Core
    Qt5::Widgets
    Qt5::DBus
    Qt5::Test
)

target_link_libraries(${UT_BIN_NAME} PRIVATE
    ${DtkWidget_LIBRARIES}
    ${GIO_LIBRARIES}
    ${DFrameworkDBus_LIBRARIES}
    Qt5::Core
    Qt5::Widgets
    Qt5::DBus
    Qt5::Test
    -lpthread
    -lgcov
    -lgtest
)
