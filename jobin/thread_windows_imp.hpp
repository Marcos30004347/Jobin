#ifndef JOBIN_THREAD_WINDOWS_H
#define JOBIN_THREAD_WINDOWS_H

#include <thread>
#ifdef WINDOWS_PLATFORM
#include <winbase.h>
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
        #ifdef WINDOWS_PLATFORM
        auto mask = (static_cast<DWORD_PTR>(1) << core);
        auto ret = SetThreadAffinityMask(_thread.native_handle(), mask);
        assert(ret);
        #endif
    }
};

#endif
