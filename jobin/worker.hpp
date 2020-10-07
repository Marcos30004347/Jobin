#ifndef JOBIN_WORKER_H
#define JOBIN_WORKER_H

#include "job.hpp"
#include "thread.hpp"
#include "fiber_pool_fixed.hpp"
#include "job_manager.hpp"

class worker;

thread_local static worker* current_worker = nullptr;

class worker {
    friend void return_to_worker();
private:
    /**
     * handler of the worker.
     */
    void(*handler)(void*);

    /**
     * voidless pointer to pass as argument to handler.
     */
    void* arg = nullptr;

    /**
     * _thread of execution of this worker.
     */
    thread* _thread = nullptr;

    /**
     * thread context of execution.
     */
    fiber* thread_fib = nullptr;

    /**
     * flag that signal if workers can stop pooling jobs.
     */
    static bool should_pool;

    /**
     * number of currently running workers.
     */
    static atomic<unsigned int> running_workers;

    /**
     * worker fiber pool.
     */
    fiber_pool_fixed<256> fiber_pool;

    /**
     * worker id.
     */
    unsigned int id = 0;


    /**
     * Main routine of the worker, this function is responsable for
     * pooling and executing jobs.
     */
    static void do_work(void *data);

    /**
     * Routine responsable for setting up a worker for
     * the do_work method.
     */
    static void init_worker(void *data);

public:
    /**
     * worker contructor.
     * @param handler: worker initial job;
     * @param arg: initial argument.
     */
    worker(void(*handler)(void*), void* arg);
    
    /**
     * worker contructor.
     * @note: this contrutor dont dispatch any initial job.
     */
    worker();
    

    /**
     * wait for current worker to return.
     */
    void wait();

    /**
     * worker destructor.
     */
    ~worker();


    /**
     * Convert current thread to a worker.
     */
    static void convert_thread_to_worker(void(*handler)(void*), void* arg);

    class all_workers {
    public:
        /**
         * Let all workers end current executing jobs and exit
         */
        static void done();
        static void begin();
    };
};


#endif
