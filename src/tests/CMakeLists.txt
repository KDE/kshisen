enable_testing()

#set(CMAKE_AUTOMOC ON)

#set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Test REQUIRED)

add_executable(movetest movetest.cpp ../move.cpp)
add_test(movetest movetest)

target_link_libraries(movetest Qt5::Test)
