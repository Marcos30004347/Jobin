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

project(atomic_test)
add_executable(atomic_test atomic.cpp)
target_link_libraries(atomic_test jobin)
add_test(NAME atomic COMMAND atomic_test)

project(thread_test)
add_executable(thread_test thread.cpp)
target_link_libraries(thread_test jobin)
add_test(NAME thread COMMAND thread_test)

project(fiber_pool_fixed_test)
add_executable(fiber_pool_fixed_test fiber_pool_fixed.cpp)
target_link_libraries(fiber_pool_fixed_test jobin)
add_test(NAME fiber_pool_fixed COMMAND fiber_pool_fixed_test)

project(job_manager_test)
add_executable(job_manager_test job_manager.cpp)
target_link_libraries(job_manager_test jobin)
add_test(NAME job_manager COMMAND job_manager_test)

# project(worker_test)
# add_executable(worker_test worker.cpp)
# target_link_libraries(worker_test jobin)
# add_test(NAME worker COMMAND worker_test)

# project(job_test)
# add_executable(job_test job.cpp)
# target_link_libraries(job_test jobin)
# add_test(NAME job COMMAND job_test)