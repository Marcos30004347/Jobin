#ifndef JOBIN_WORKER_H
#define JOBIN_WORKER_H

#include "thread.hpp"
#include "job.hpp"
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
     * global job manager.
     */
    job_manager* global_manager = nullptr;

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
    static void do_work(void *data) {
        worker* me = reinterpret_cast<worker*>(data);
        promise<void>* promise_h = nullptr;
        if(me->handler && me->global_manager) { promise_h = me->global_manager->enqueue_job<void, void*>(me->handler, me->arg);}
    
        do { me->id = worker::running_workers.load(); } while(!worker::running_workers.compare_exchange_weak(me->id, me->id+1));
        
        
        // while(worker::running_workers.load() < thread::hardware_concurrency()) {}

        thread::this_thread::set_affinity(me->id);
    
        while(worker::should_pool && me->global_manager) {
            while(me->global_manager->jobs.dequeue(current_job)) {
                if(current_job->status == job_status::UNINITIATED) {
                    current_job->pool = &me->fiber_pool;
                    current_job->context = me->fiber_pool.request();
                    reset_fiber(current_job->context, job::execute, current_job);
                    current_job->status = job_status::RUNNING;
                } else
                if(current_job->status == job_status::WAITING) {

                    current_job->status = job_status::RUNNING;
                }

                // printf("worker %i is executing job %i...\n", me->id, current_job);
                current_job->lock.lock();
                switch_to_fiber(me->thread_fib, current_job->context);
                // printf("worker %i return from job %p...\n", me->id, current_job);
                current_job->lock.unlock();

                if(current_job->status == job_status::FINISHING) {
                    current_job->pool->release(current_job->context);
                    delete current_job;
                }

            }
        }
        if(promise_h) delete promise_h;
        // printf("worker %i is finishing...\n", me->id);
    }

    /**
     * Routine responsable for setting up a worker for
     * the do_work method.
     */
    static void init_worker(void *data) {
        worker* me = reinterpret_cast<worker*>(data);
        current_worker = me;
        me->thread_fib = create_fiber();
        me->global_manager = job_manager::get_ptr();
        convert_thread_to_fiber(me->thread_fib, worker::do_work, data);
        destroy_fiber(me->thread_fib);
    }

public:
    /**
     * worker contructor.
     * @param handler: worker initial job;
     * @param arg: initial argument.
     */
    worker(void(*handler)(void*), void* arg): arg{arg}, handler{handler}, thread_fib{nullptr}, fiber_pool{}, _thread{nullptr} {
        this->_thread = new thread(worker::init_worker, this);
    }
    
    /**
     * worker contructor.
     * @note: this contrutor dont dispatch any initial job.
     */
    worker(): arg{nullptr}, handler{nullptr}, thread_fib{nullptr}, fiber_pool{}, _thread{nullptr} {}
    

    /**
     * wait for current worker to return.
     */
    void wait() { _thread->join(); }

    /**
     * worker destructor.
     */
    ~worker() {
        delete _thread;
    }


    /**
     * Convert current thread to a worker.
     */
    static void convert_thread_to_worker(void(*handler)(void*), void* arg) {
        worker* me = new worker;
        me->handler = handler;
        me->arg = arg;
        worker::init_worker(me);
        delete me;
    }

    class all_workers {
    public:
        /**
         * Let all workers end current executing jobs and exit
         */
        static void done() {
            should_pool = false;
        }
    };
};

bool worker::should_pool = true;

atomic<unsigned int> worker::running_workers{0};

void return_to_worker() {
    switch_to_fiber(get_current_fiber(), current_worker->thread_fib);
}

void notify_caller(job* job) {
    if(job->waiting_for_me)
    job_manager::get_ptr()->notify(job->waiting_for_me);
}

void current_job_yield() {
    job_manager::get_ptr()->jobs.enqueue(current_job);
    return_to_worker();
}

#endif
