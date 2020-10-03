#ifndef JOBIN_CONFIG_H
#define JOBIN_CONFIG_H

/*
    fcontext current suported platforms

    Windows (x86_64, Win32)
    Linux (x86_64/x86)
    OSX (x86_64/x86)
    Android (ARM/x86/ARM64/x86_64)
    iOS (Arm64, Arm7, x86_64, i386)
*/

// TODO: Better definition of macros

#if __EMSCRIPTEN__
#define EMSCRIPTEN_PLATFORM
#define EMSCRIPTEN_FIBER_BACKEND
#elif defined(__linux__) && !defined(__ANDROID__)
    #define THREAD_PTHREAD_BACKEND
    #define POSIX_PLATFORM

    #ifdef __x86_64__
        #define FIBER_FCONTEXT_BACKEND
    #elif defined(i386) || defined(__i386) || defined(__i386__)
        #define FIBER_FCONTEXT_BACKEND 
    #endif
#elif __ANDROID__
    // #define THREAD_PTHREAD_BACKEND
    #define POSIX_PLATFORM

    #ifdef __arm__   //ARM
        #define FIBER_FCONTEXT_BACKEND
    #elif __aarch64__ //ARM64
        #define FIBER_FCONTEXT_BACKEND
    #elif __x86_64__ //x86_64
        #define FIBER_FCONTEXT_BACKEND
    #elif defined(__amd64) || defined(__amd64__) || defined(__x86_64)  // AMD64
        #define FIBER_FCONTEXT_BACKEND
    #elif defined(i386) || defined(__i386) || defined(__i386__) //x86
        #define FIBER_FCONTEXT_BACKEND
    #elif
        #error Unknown Android platform
    #endif
#elif __APPLE__
    #define THREAD_PTHREAD_BACKEND
    #define POSIX_PLATFORM

    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR
        #define FIBER_FCONTEXT_BACKEND
    #elif TARGET_OS_IPHONE
        #define FIBER_FCONTEXT_BACKEND
    #elif TARGET_OS_MAC
        #ifdef __x86_64__
            #define FIBER_FCONTEXT_BACKEND
        #elif defined(i386) || defined(__i386) || defined(__i386__)
            #define FIBER_FCONTEXT_BACKEND 
        #elif
            #error "Unknown OSX platform"
        #endif
    #elif
        #error "Unknown Apple platform"
    #endif

#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    #define WINDOWS_PLATFORM
    #define THREAD_WINDOWS_BACKEND
    #define FIBER_WINDOWS_BACKEND
#elif __unix__
    #define POSIX_PLATFORM
    #define THREAD_PTHREAD_BACKEND
    #define FIBER_CROSS_BACKEND
#endif

#endif