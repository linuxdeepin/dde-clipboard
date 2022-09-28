set(BIN_NAME dde-clipboard-daemon)

file(GLOB_RECURSE SRCS "*.h" "*.cpp")

add_executable(${BIN_NAME}
    ${SRCS}
    )

target_include_directories(${BIN_NAME} PUBLIC
    ${Qt5Widget_INCLUDE_DIRS}
    ${Qt5Core_INCLUDE_DIRS}
    ${DtkCore_INCLUDE_DIRS}
    KF5::WaylandClient
    )

target_link_libraries(${BIN_NAME} PRIVATE
    ${Qt5DBus_LIBRARIES}
    ${Qt5Core_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${DtkCore_LIBRARIES}
    KF5::WaylandClient
    )

install(TARGETS ${BIN_NAME} DESTINATION bin)
