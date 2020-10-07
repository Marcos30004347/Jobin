#include "worker.hpp"
#include "job_manager.hpp"
#include "fiber_pool_fixed.hpp"

bool worker::should_pool = false;

atomic<unsigned int> worker::running_workers{0};

/**
 * thread context of execution.
 */
thread_local fiber* thread_fib = nullptr;

/**
 * worker fiber pool.
 */
thread_local fiber_pool_fixed<256> fiber_pool;

void worker::do_work(void *data) {
    worker* me = reinterpret_cast<worker*>(data);
    promise<void> promise_h;
    promise<void> promise_c;
    if(me->handler) {
        job_manager::enqueue_job<void, void*>(&promise_h, me->handler, me->arg);
    }

    do {
        me->id = worker::running_workers.load();
    } while(!worker::running_workers.compare_exchange_weak(me->id, me->id+1));
    
    // while(worker::running_workers.load() < thread::hardware_concurrency()) {}

    thread::this_thread::set_affinity(me->id % thread::hardware_concurrency());

    job* next_job = nullptr;

    while (!should_pool){}

    while(worker::should_pool) {
        while(job_manager::queues.dequeue(next_job)) {
            set_current_job(next_job);
            if(get_current_job()->status == job_status::UNINITIATED) {
                get_current_job()->pool = &fiber_pool;
                get_current_job()->context = fiber_pool.request();
                reset_fiber(get_current_job()->context, job::execute, get_current_job());
                get_current_job()->status = job_status::RUNNING;
            } else
            if(get_current_job()->status == job_status::WAITING) {
                get_current_job()->status = job_status::RUNNING;
            }

            // printf("worker %i is executing job %i...\n", me->id, current_job);
            get_current_job()->lock.lock();

            // std::cout << current_job->context << std::endl;
        
            switch_to_fiber(thread_fib, get_current_job()->context);

            // printf("worker %i return from job %p...\n", me->id, get_current_job());
            get_current_job()->lock.unlock();
            if(get_current_job()->status == job_status::FINISHING) {
                get_current_job()->pool->release(get_current_job()->context);
                delete get_current_job();
            }
        }
    }
    // printf("worker %i is finishing...\n", me->id);
}

void worker::init_worker(void *data) {

    worker* me = reinterpret_cast<worker*>(data);
    current_worker = me;
    thread_fib = create_fiber();
    convert_thread_to_fiber(thread_fib, worker::do_work, data);
    destroy_fiber(thread_fib);
}

worker::worker(void(*handler)(void*), void* arg): arg{arg}, handler{handler}, _thread{nullptr} {
    this->_thread = new thread(worker::init_worker, this);
}
    

worker::worker(): arg{nullptr}, handler{nullptr}, _thread{nullptr} {}
    


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
    worker::should_pool = false;
}

void worker::all_workers::begin() {
    worker::should_pool = true;
}

void return_to_worker() {
    switch_to_fiber(get_current_fiber(), thread_fib);
}

void notify_caller(job* job) {
    if(job->waiting_for_me) job_manager::notify(job->waiting_for_me);
}

void current_job_yield() {
    job_manager::queues.enqueue(get_current_job());
    return_to_worker();
}

