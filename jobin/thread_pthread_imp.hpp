#ifndef JOBIN_THREAD_PTHREAD_H
#define JOBIN_THREAD_PTHREAD_H

#include <thread>
#ifdef POSIX_PLATFORM
#include <pthread.h>
#endif

#include <assert.h>

class thread {
    private:
    std::thread _thread;
    public:

    static unsigned int hardware_concurrency() {
        return std::thread::hardware_concurrency();
    }

    thread(void (*fn)(void* data), void* data): _thread(fn, data) {}
    ~thread() { _thread.~thread(); }

    inline void join() { _thread.join(); }

    inline void close() { this->~thread(); }

    inline void get_id() { _thread.get_id(); }
    inline void swap_with(thread& worker) { _thread.swap(worker._thread); }

    void set_affinity(int core) {
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(core, &cpuset);

        pthread_t current_thread = pthread_self();
        int error = pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
        assert(!error);
    }

};

#endif