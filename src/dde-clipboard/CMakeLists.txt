set(BIN_NAME dde-clipboard)

generation_dbus_adaptor(
    ${CMAKE_SOURCE_DIR}/src/dde-clipboard/com.deepin.dde.Clipboard.xml
    ClipboardAdaptor
    ${CMAKE_CURRENT_BINARY_DIR}/clipboard_adaptor
    -N
    )

set(DBUS_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/clipboard_adaptor.h
    ${CMAKE_CURRENT_BINARY_DIR}/clipboard_adaptor.cpp
    )

file(GLOB_RECURSE SRCS "*.h" "*.cpp")

add_executable(${BIN_NAME}
    ${SRCS}
    ${DBUS_SRCS}
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

## bin
install(TARGETS ${BIN_NAME} DESTINATION bin)
