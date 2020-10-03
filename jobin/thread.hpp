#ifndef JOBIN_THREAD_H
#define JOBIN_THREAD_H

#include "config.hpp"

#include <thread>
#ifdef THREAD_PTHREAD_BACKEND
#include <pthread.h>
#elif THREAD_WINDOWS_BACKEND
#include <winbase.h>
#endif

#include <assert.h>

#if defined(ANDROID) || defined(__ANDROID__)
#include<sched.h>
#endif

class thread {
    private:
    
    std::thread _thread;
    public:

    /**
     *   Return the number of hardware threads.
     */
    static unsigned int hardware_concurrency() {
        return std::thread::hardware_concurrency();
    }

    thread(void (*fn)(void* data), void* data): _thread(fn, data) {}
    ~thread() { _thread.~thread(); }

    /**
     *   Wait thread to join.
     */
    inline void join() { _thread.join(); }

    /**
     *   Close running thread.
     */
    inline void close() { this->~thread(); }

    /**
     *   Returns thread id.
     */
    inline void get_id() { _thread.get_id(); }

    /**
     *   Swap the state of a thread with the state of @other thread.
     */
    inline void swap_with(thread& other) { _thread.swap(other._thread); }


    class this_thread {
    public:
        /**
         * Set current running thread affinity to @core hardware core/thread.
         * @param core: hardware core/thread id to set affinity to.
         */
        static bool set_affinity(int core) {
        
            #ifdef THREAD_PTHREAD_BACKEND
            // pthread specific code
            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            CPU_SET(core, &cpuset);

            pthread_t current_thread = pthread_self();
            int error = pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
            if(error) return false;
            return true;
        
            #elif THREAD_WINDOWS_BACKEND
            // WinThread specific code
            auto mask = (static_cast<DWORD_PTR>(1) << core);
            auto ret = SetThreadAffinityMask(_thread.native_handle(), mask);
            if(!ret) return false;
            return true;

            #elif defined(ANDROID) || defined(__ANDROID__)
            cpu_set_t my_set;
            CPU_ZERO(&my_set);
            CPU_SET(7, &my_set);
            sched_setaffinity(0, sizeof(cpu_set_t), &my_set);
            #endif
        }

        /**
         * Yield's current thread execution, allowing another thread to run.
         */
        static void yield() {
            std::this_thread::yield();
        }
    };

};

#endif
