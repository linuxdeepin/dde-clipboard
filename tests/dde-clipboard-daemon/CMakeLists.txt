set(BIN_NAME ut-dde-clipboard-daemon)

file(GLOB_RECURSE SRCS "*.h" "*.cpp"
    "../../src/dde-clipboard-daemon/*")

list(REMOVE_ITEM SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../../src/dde-clipboard-daemon/main.cpp")

add_executable(${BIN_NAME}
    ${SRCS}
    )

# 用于测试覆盖率的编译条件
target_compile_options(${BIN_NAME} PRIVATE -fprofile-arcs -ftest-coverage)

target_include_directories(${BIN_NAME} PUBLIC
    ${Qt5Widget_INCLUDE_DIRS}
    ${Qt5Core_INCLUDE_DIRS}
    ${DtkCore_INCLUDE_DIRS}
    KF5::WaylandClient
    Qt5::Test
    )

target_link_libraries(${BIN_NAME} PRIVATE
    ${Qt5DBus_LIBRARIES}
    ${Qt5Core_LIBRARIES}
    ${Qt5Widgets_LIBRARIES}
    ${DtkCore_LIBRARIES}
    KF5::WaylandClient
    Qt5::Test
    -lpthread
    -lgcov
    -lgtest
    )
