cmake_minimum_required(VERSION 3.16)

set(BIN_NAME dde-clipboard)

project(${BIN_NAME})

#set(CMAKE_VERBOSE_MAKEFILE ON)
set(QT_VERSION_MAJOR 6)
set(CMAKE_CXX_STANDARD 17)
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

# Install settings
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX /usr)
endif ()

include(GNUInstallDirs)

if (NOT (${CMAKE_BUILD_TYPE} MATCHES "Debug"))
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Ofast")
endif ()

# Find the library
find_package(PkgConfig REQUIRED)
find_package(Dtk6 COMPONENTS Widget Core Tools REQUIRED)
find_package(Qt6 COMPONENTS Core Gui Widgets DBus WaylandClient Test Concurrent LinguistTools REQUIRED)
find_package(DDEShell REQUIRED)
find_package(DdeTrayLoader REQUIRED)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

pkg_check_modules(GIO REQUIRED IMPORTED_TARGET gio-qt6)

if (NOT DEFINED SYSTEMD_USER_UNIT_DIR)
    pkg_get_variable(SYSTEMD_USER_UNIT_DIR systemd systemduserunitdir)
endif()

include_directories(
    dde-clipboard
    dde-clipboard/dbus
    dde-clipboard/displaymanager
    dde-clipboardloader
)

#----------------------------dde-clipboard------------------------------

qt_add_dbus_adaptor(DBUS_INTERFACES ${CMAKE_SOURCE_DIR}/dde-clipboard/org.deepin.dde.Clipboard1.xml mainwindow.h MainWindow)
dtk_add_dbus_interface(DBUS_INTERFACES ${CMAKE_SOURCE_DIR}/dbus/org.deepin.dde.Display1.Monitor.xml Monitor)
dtk_add_dbus_interface(DBUS_INTERFACES ${CMAKE_SOURCE_DIR}/dbus/org.deepin.dde.Display1.xml Display1)
dtk_add_dbus_interface(DBUS_INTERFACES ${CMAKE_SOURCE_DIR}/dbus/org.deepin.dde.daemon.Dock1.xml Dock1)

include_directories(
    ${PROJECT_SOURCE_DIR}/dbus/interface
    ${PROJECT_SOURCE_DIR}/dbus/types
    ${PROJECT_SOURCE_DIR}/dbus
)

aux_source_directory(dbus/types DBUS_TYPES)

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
    Dtk6::Widget
    Dtk6::Core
    PkgConfig::GIO
    Qt6::Core
    Qt6::Widgets
    Qt6::GuiPrivate
    Qt6::Gui
    Qt6::DBus
    Dde::Shell
)

# generate qm
set(TRANSLATION_FILES
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_af.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_am_ET.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_ar.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_ast.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_az.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_bg.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_bn.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_bo.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_ca.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_cs.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_da.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_de.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_el_GR.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_el.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_en_AU.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_en_GB.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_eo.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_es_419.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_es.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_et.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_fa.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_fi.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_fr.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_gl_ES.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_he.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_hi_IN.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_hi.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_hr.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_hu.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_hy.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_id.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_it.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_ja.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_kab.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_ko.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_ku_IQ.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_ku.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_ky@Arab.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_lt.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_lv.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_ml.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_mn.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_ms.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_nb.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_ne.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_nl.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_pa.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_pl.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_pt_BR.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_pt.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_ro.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_ru.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_si.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_sk.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_sl.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_sq.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_sr.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_sv.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_sw.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_ta.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_tr.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_ug.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_uk.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_vi.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_zh_CN.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_zh_HK.ts
    ${PROJECT_SOURCE_DIR}/translations/dde-clipboard_zh_TW.ts
)

qt_add_translations(${BIN_NAME}
    TS_FILES ${TRANSLATION_FILES}
    SOURCES ${Clipboard_SCRS}
    QM_FILES_OUTPUT_VARIABLE TRANSLATED_FILES
)

macro(install_symlink filepath wantsdir)
    file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/link/${wantsdir}/)
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${SYSTEMD_USER_UNIT_DIR}/${filepath} ${PROJECT_BINARY_DIR}/link/${wantsdir}/${filepath})
    install(FILES ${PROJECT_BINARY_DIR}/link/${wantsdir}/${filepath} DESTINATION ${SYSTEMD_USER_UNIT_DIR}/${wantsdir}/)
endmacro(install_symlink)

## qm files
install(FILES ${TRANSLATED_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/${BIN_NAME}/translations)

configure_file(
    misc/dde-clipboard.service.in
    dde-clipboard.service
    @ONLY
)

configure_file(
    misc/org.deepin.dde.Clipboard1.service.in
    org.deepin.dde.Clipboard1.service
    @ONLY
)

## dbus service
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.deepin.dde.Clipboard1.service DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/dde-clipboard.service DESTINATION ${SYSTEMD_USER_UNIT_DIR})

install_symlink(dde-clipboard.service dde-session-initialized.target.wants)

## bin
install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})

#----------------------------dde-clipboard-daemon------------------------------
set(BIN_NAME dde-clipboard-daemon)

file(GLOB_RECURSE dde-clipboard-daemon_SCRS
        "dde-clipboard-daemon/*.h"
        "dde-clipboard-daemon/*.cpp"
)

add_executable(${BIN_NAME}
    ${dde-clipboard-daemon_SCRS}
)

qt_generate_wayland_protocol_client_sources(${BIN_NAME} FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/dde-clipboard-daemon/protocol/wlr-data-control-unstable-v1.xml)

target_link_libraries(${BIN_NAME} PRIVATE
  Qt6::Gui
  Qt6::DBus
  Qt6::Concurrent
  Qt6::WaylandClient
  Qt6::WaylandClientPrivate
  Dtk6::Core
)

install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})

configure_file(
    misc/org.deepin.dde.ClipboardLoader1.service.in
    org.deepin.dde.ClipboardLoader1.service
    @ONLY
)

configure_file(
    misc/org.deepin.dde.daemon.Clipboard1.service.in
    org.deepin.dde.daemon.Clipboard1.service
    @ONLY
)
configure_file(
    misc/dde-clipboard-daemon.service.in
    dde-clipboard-daemon.service
    @ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.deepin.dde.ClipboardLoader1.service
    DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services
)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.deepin.dde.daemon.Clipboard1.service
    DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/dde-clipboard-daemon.service DESTINATION ${SYSTEMD_USER_UNIT_DIR})

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
    Dtk6::Widget
    PkgConfig::GIO
    Qt6::Core
    Qt6::Widgets
    Qt6::DBus
    Qt6::Test
    Qt6::GuiPrivate
    Dde::Shell
    -lpthread
    -lgcov
    -lgtest
)

#--------------------------dock-plugin---------------------------
set(PLUGIN_NAME dock-clipboard-plugin)

file(GLOB_RECURSE SRCS
    "dock-clipboard-plugin/*.h"
    "dock-clipboard-plugin/*.cpp"
    "dock-clipboard-plugin/widget/*.h"
    "dock-clipboard-plugin/widget/*.cpp"
)

add_definitions("${QT_DEFINITIONS} -DQT_PLUGIN")
add_library(${PLUGIN_NAME} SHARED ${SRCS} dock-clipboard-plugin/resources/clipboard.qrc)
target_include_directories(${PLUGIN_NAME} PUBLIC
    ${DtkWidget_INCLUDE_DIRS}
    ${DtkCore_INCLUDE_DIRS}
    ${OBJECT_BINARY_DIR}
    ${DDE_DOCK_INCLUDE_DIR}
    ${QGSettings_INCLUDE_DIRS}
    ${DFrameworkDBus_INCLUDE_DIRS}
    dock-clipboard-plugin/widget/
)
target_link_libraries(${PLUGIN_NAME} PRIVATE
    ${DtkWidget_LIBRARIES}
    ${QGSettings_LIBRARIES}
    ${DFrameworkDBus_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${Qt5Svg_LIBRARIES}
    ${DtkCore_LIBRARIES}
)

# 设置执行 make install 时哪个目标应该被 install 到哪个位置
install(TARGETS ${PLUGIN_NAME} LIBRARY DESTINATION lib/dde-dock/plugins)
install(FILES "dock-clipboard-plugin/resources/clipboard.svg" DESTINATION share/dde-dock/icons/dcc-setting)
