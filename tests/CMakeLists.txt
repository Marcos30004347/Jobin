cmake_minimum_required(VERSION 3.10)

include(CTest)

project(fiber_test)
add_executable(fiber_test fiber.cpp)
target_link_libraries(fiber_test jobin)
add_test(NAME fiber COMMAND fiber_test)