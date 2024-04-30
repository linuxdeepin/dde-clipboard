
# Find the library
find_package(Qt6 COMPONENTS Core Gui Widgets DBus Test Concurrent WaylandClient REQUIRED)

find_package(Qt6WaylandScannerTools REQUIRED)

set(BIN_NAME dde-clipboard-daemon)

add_executable(${BIN_NAME})

set(dde-clipboard-daemon_SCRS
    main.cpp
    clipboarddaemon.h
    clipboarddaemon.cpp
    clipboardloader.h
    clipboardloader.cpp
    waylandcopyclient.h
    waylandcopyclient.cpp
)

qt_generate_wayland_protocol_client_sources(dde-clipboard-daemon FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/../misc/wlr-data-control-unstable-v1.xml
)

target_sources(${BIN_NAME} PRIVATE
    ${dde-clipboard-daemon_SCRS}
)

target_link_libraries(${BIN_NAME} PRIVATE
    Qt::Gui
    Qt::DBus
    Qt::Widgets
    Qt::Concurrent
    Qt::WaylandClient
    Qt::WaylandClientPrivate
)

install(TARGETS ${BIN_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
