#include "worker.hpp"

bool worker::should_pool = false;
atomic<unsigned int> worker::running_workers{0};

void worker::do_work(void *data) {
    worker* me = reinterpret_cast<worker*>(data);
    promise<void> promise_h;
    if(me->handler) { job_manager::get_singleton_ptr()->enqueue_job<void, void*>(&promise_h, me->handler, me->arg);}

    do { me->id = worker::running_workers.load(); } while(!worker::running_workers.compare_exchange_weak(me->id, me->id+1));
    
    
    // while(worker::running_workers.load() < thread::hardware_concurrency()) {}

    thread::this_thread::set_affinity(me->id % thread::hardware_concurrency());

    while (!should_pool){}

    while(worker::should_pool) {
        while(job_manager::get_singleton_ptr()->queues.dequeue(current_job)) {
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
    // printf("worker %i is finishing...\n", me->id);
}

void worker::init_worker(void *data) {

    worker* me = reinterpret_cast<worker*>(data);
    current_worker = me;
    me->thread_fib = create_fiber();
    convert_thread_to_fiber(me->thread_fib, worker::do_work, data);
    destroy_fiber(me->thread_fib);
}

worker::worker(void(*handler)(void*), void* arg): arg{arg}, handler{handler}, thread_fib{nullptr}, fiber_pool{}, _thread{nullptr} {
    this->_thread = new thread(worker::init_worker, this);
}
    

worker::worker(): arg{nullptr}, handler{nullptr}, thread_fib{nullptr}, fiber_pool{}, _thread{nullptr} {}
    


void worker::wait() {
    worker::running_workers.compare_exchange_strong(worker::running_workers.load(), worker::running_workers.load() - 1);
    _thread->join();
    delete _thread;
}


worker::~worker() {}


void worker::convert_thread_to_worker(void(*handler)(void*), void* arg) {
    worker::all_workers::begin();
    worker* me = new worker;
    me->handler = handler;
    me->arg = arg;
    worker::init_worker(me);
    delete me;
}


void worker::all_workers::done() {
    should_pool = false;
}

void worker::all_workers::begin() {
    should_pool = true;
}

void return_to_worker() {
    switch_to_fiber(get_current_fiber(), current_worker->thread_fib);
}

void notify_caller(job* job) {
    if(job->waiting_for_me)
    job_manager::get_singleton_ptr()->notify(job->waiting_for_me);
}

void current_job_yield() {
    job_manager::get_singleton_ptr()->queues.enqueue(current_job);
    return_to_worker();
}

