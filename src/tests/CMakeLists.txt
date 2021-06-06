find_package(Qt5Test ${QT_MIN_VERSION} REQUIRED)

add_executable(movetest movetest.cpp ../move.cpp)
add_test(NAME movetest COMMAND movetest)

target_link_libraries(movetest Qt::Test)
