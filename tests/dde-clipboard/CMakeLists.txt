set(BIN_NAME ut-dde-clipboard)

file(GLOB_RECURSE SRCS "*.h" "*.cpp"
    "../../src/dde-clipboard/*")

list(REMOVE_ITEM SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../../src/dde-clipboard/main.cpp")

add_executable(${BIN_NAME}
    ${SRCS}
    )

# 用于测试覆盖率的编译条件
target_compile_options(${BIN_NAME} PRIVATE -fprofile-arcs -ftest-coverage)

target_include_directories(${BIN_NAME} PUBLIC
    ${DtkWidget_INCLUDE_DIRS}
    ${GIO_INCLUDE_DIRS}
    ${DFrameworkDBus_INCLUDE_DIRS}
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    Qt5::Core
    Qt5::Widgets
    Qt5::DBus
    Qt5::Test
    )

target_link_libraries(${BIN_NAME} PRIVATE
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
