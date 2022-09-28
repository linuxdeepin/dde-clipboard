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
find_package(KF5Wayland REQUIRED)
find_package(Qt5 COMPONENTS Core Widgets DBus Test REQUIRED)

pkg_check_modules(DFrameworkDBus REQUIRED dframeworkdbus)
pkg_check_modules(GIO REQUIRED gio-qt)

try_compile(USE_DEEPIN_KF5_WAYLAND
    ${CMAKE_CURRENT_BINARY_DIR}/try_compile
    SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/kf5_wayland_test.cc
    LINK_LIBRARIES
    KF5::WaylandClient
    )

message(STATUS "USE_DEEPIN_KF5_WAYLAND ${USE_DEEPIN_KF5_WAYLAND}")

if(USE_DEEPIN_KF5_WAYLAND)
    add_definitions(-DUSE_DEEPIN_KF5_WAYLAND)
endif()

function(generation_dbus_adaptor xml class_name class_file option)
    execute_process(COMMAND qdbusxml2cpp ${option} -a ${class_file} -c ${class_name} ${xml}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endfunction(generation_dbus_adaptor)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}/src/dde-clipboard
    src/dde-clipboard
    src/dde-clipboard/dbus
    src/dde-clipboard/displaymanager
    src/dde-clipboard-daemon
    )

add_subdirectory("src/dde-clipboard")
add_subdirectory("src/dde-clipboard-daemon")
add_subdirectory("tests/dde-clipboard")
add_subdirectory("tests/dde-clipboard-daemon")

## qm files
file(GLOB QM_FILES "translations/*.qm")
install(FILES ${QM_FILES} DESTINATION share/${BIN_NAME}/translations)

## desktop file
install(FILES "misc/dde-clipboard.desktop"
    DESTINATION share/applications)

## autostart
install(FILES "misc/dde-clipboard.desktop"
    DESTINATION /etc/xdg/autostart)

install(FILES misc/dde-clipboard-daemon.desktop
    DESTINATION /etc/xdg/autostart/
    )

## dbus service
install(FILES misc/com.deepin.dde.Clipboard.service
    DESTINATION share/dbus-1/services)

install(FILES misc/dde-clipboard-daemon.service
    DESTINATION lib/systemd/user
    )

install(FILES misc/dde-clipboard-daemon.desktop
    DESTINATION share/applications/
    )

