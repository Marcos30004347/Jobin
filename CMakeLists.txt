cmake_minimum_required(VERSION 3.10)

project(_jobin)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_EXTENSIONS OFF)

add_library(jobin INTERFACE)
target_include_directories(jobin INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if (${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
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
elseif()

add_subdirectory(third-party/cross-fiber)
add_subdirectory(third-party/fcontext)
find_package(Threads REQUIRED)

target_link_libraries(jobin INTERFACE cross-fiber)
target_link_libraries(jobin INTERFACE fcontext)
target_link_libraries(jobin INTERFACE Threads::Threads)

elseif (ANDROID)
# Android

message("Compiling for Android!")

    add_subdirectory(third-party/fcontext)
    find_package(Threads REQUIRED)
    target_link_libraries(jobin INTERFACE Threads::Threads)
    target_link_libraries(jobin INTERFACE fcontext)

    if (${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm")
        set(CPU_ARCH "arm")
        set(ASM_EXT "aapcs_elf_gas.S")
    elseif (${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64")
        set(CPU_ARCH "arm64")
        set(ASM_EXT "aapcs_elf_gas.S")
    elseif (${CMAKE_SYSTEM_PROCESSOR} MATCHES "i686")
        set(CPU_ARCH "i386")
        set(ASM_EXT "sysv_elf_gas.S")
    elseif (${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
        set(CPU_ARCH "x86_64")
        set(ASM_EXT "sysv_elf_gas.S")
    endif()
endif()

if(TESTS)
enable_testing()
add_subdirectory(tests)
endif()