cmake_minimum_required(VERSION 3.10)

project(jobin)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_EXTENSIONS OFF)


if(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")

set(USE_FLAGS "${USE_FLAGS} -o3")
set(USE_FLAGS "${USE_FLAGS} -std=c++17")
set(USE_FLAGS "${USE_FLAGS} -s WASM=1")
set(USE_FLAGS "${USE_FLAGS} -s ASYNCIFY=1")
set(USE_FLAGS "${USE_FLAGS} -s USE_PTHREADS=1")
set(USE_FLAGS "${USE_FLAGS} -s PTHREAD_POOL_SIZE=8")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${USE_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${USE_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${USE_FLAGS}")
set(CMAKE_EXECUTABLE_SUFFIX .js)

else()

add_subdirectory(third-party/cross-fiber)
add_subdirectory(third-party/fcontext)

find_package(Threads REQUIRED)

link_libraries(cross-fiber)
link_libraries(fcontext)
link_libraries(Threads::Threads)

endif()

add_library(jobin STATIC
jobin/fiber.cpp
jobin/mutex.cpp
jobin/spin_lock.cpp
jobin/thread.cpp
jobin/worker.cpp
jobin/job_manager.cpp
)

target_include_directories(jobin PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(TESTS)
enable_testing()
add_subdirectory(tests)
endif()