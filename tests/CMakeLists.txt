cmake_minimum_required(VERSION 3.10)

include(CTest)

project(fiber_test)
add_executable(fiber_test fiber.cpp)
target_link_libraries(fiber_test jobin)
add_test(NAME fiber COMMAND fiber_test)

project(promise_test)
add_executable(promise_test promise.cpp)
target_link_libraries(promise_test jobin)
add_test(NAME promise COMMAND promise_test)

project(promise_resolver_test)
add_executable(promise_resolver_test promise_resolver.cpp)
target_link_libraries(promise_resolver_test jobin)
add_test(NAME promise_resolver COMMAND promise_resolver_test)