find_package(Qt${QT_MAJOR_VERSION}Test ${QT_MIN_VERSION} REQUIRED)

add_executable(movetest movetest.cpp movetest.h ../move.cpp ../move.h)
add_test(NAME movetest COMMAND movetest)

target_link_libraries(movetest Qt${QT_MAJOR_VERSION}::Test)
