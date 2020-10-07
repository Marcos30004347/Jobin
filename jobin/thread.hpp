#ifndef JOBIN_THREAD_H
#define JOBIN_THREAD_H

#include <thread>

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

    thread(void (*fn)(void* data), void* data);
    ~thread();

    /**
     *   Wait thread to join.
     */
    void join() noexcept;

    /**
     *   Close running thread.
     */
    void close() noexcept;

    /**
     *   Returns thread id.
     */
    void get_id() noexcept;

    /**
     *   Swap the state of a thread with the state of @other thread.
     */
    void swap_with(thread& other) noexcept;


    class this_thread {
    public:
        /**
         * Set current running thread affinity to @core hardware core/thread.
         * @param core: hardware core/thread id to set affinity to.
         */
        static bool set_affinity(int core);

        /**
         * Yield's current thread execution, allowing another thread to run.
         */
        static void yield();
    };

};

#endif
