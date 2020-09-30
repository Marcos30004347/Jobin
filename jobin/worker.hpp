#ifndef JOBIN_WORKER_H
#define JOBIN_WORKER_H

#include "thread.hpp"

#include "job.hpp"

#include "fiber_pool_fixed.hpp"

#include "job_manager.hpp"

class worker;

thread_local static worker* current_worker = nullptr;

enum workers_sate {
    ALL_WORKERS_WAITING,
    ALL_WORKERS_RUNNING,
    ALL_WORKERS_ENDING,
};

class worker {
    private:
    int(*handler)(void*);

    void* data;

    thread _thread;

    fiber* thread_fib;

    static workers_sate gState;
    static atomic<unsigned int> running_workers;
    job_manager* global_manager;

    fiber_pool_fixed<256> fiber_pool;

    unsigned int id;

    static void do_work(void *data) {
        worker* me = reinterpret_cast<worker*>(data);

        current_job = (job*)malloc(sizeof(job));
        current_job->context = me->thread_fib;
        current_job->waiting_for_me = nullptr;

        if(me->handler) {
            job_manager::get_ptr()->enqueue_job<int, void*>(me->handler, me->data);
        }

        do { me->id = worker::running_workers.load(); } while(!worker::running_workers.compare_exchange_weak(me->id, me->id+1));
        
        // printf("worker %i is waiting for the others waiting...\n", me->id);
        
        while(worker::running_workers.load() < thread::hardware_concurrency()) {}
        
        current_worker->set_affinity(me->id);
        // printf("worker %i is running...\n", me->id);
        while(gState != ALL_WORKERS_ENDING) {
            while(me->global_manager->high_priority_queue.dequeue(current_job) || me->global_manager->medium_priority_queue.dequeue(current_job) || me->global_manager->low_priority_queue.dequeue(current_job)) {

                if(current_job->status == job_status::UNINITIATED) {
                    current_job->pool = &me->fiber_pool;
                    while(!me->fiber_pool.request(current_job->context, job::execute, current_job)){}
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
                    free(current_job);
                }

            }
        }

        // printf("worker %i is finishing...\n", me->id);
    }

    static void init_worker(void *data) {
        worker* me = reinterpret_cast<worker*>(data);
        current_worker = me;
        me->thread_fib = new fiber;
        me->global_manager = job_manager::get_ptr();

        convert_thread_to_fiber(me->thread_fib, worker::do_work, data);
        destroy_fiber(me->thread_fib);
    }

    public:
    worker(int(*handler)(void*), void* data): data{data}, handler{handler}, thread_fib{nullptr}, fiber_pool{}, _thread{worker::init_worker, this } {}
    worker(): data{nullptr}, handler{nullptr}, thread_fib{nullptr}, fiber_pool{}, _thread{worker::init_worker, this } {}
    
    void wait() {_thread.join();}
    void set_affinity(int i) { _thread.set_affinity(i); }

    static void begin() {
        gState = ALL_WORKERS_RUNNING;
    }


    static void done() {
        gState = ALL_WORKERS_ENDING;
    }

    ~worker() {
        this->_thread.~thread();
        // delete this->fiber_pool;
        // if(this->thread_fib) delete this->thread_fib;
    }

    friend void return_to_worker();
};

workers_sate worker::gState = ALL_WORKERS_WAITING;
atomic<unsigned int> worker::running_workers{0};

void return_to_worker() {
    switch_to_fiber(get_current_fiber(), current_worker->thread_fib);
}

void notify_caller(job* job) {
    if(job->waiting_for_me)
    job_manager::get_ptr()->notify(job->waiting_for_me);
}


#endif
