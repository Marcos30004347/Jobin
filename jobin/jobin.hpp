#ifndef JOBIN_SYSTEM_H
#define JOBIN_SYSTEM_H

#include<iostream>
#include "worker.hpp"
#include "job_manager.hpp"

namespace jobin {

namespace {
    worker** workers = nullptr;
    unsigned int workers_count = 0;
};

template<typename T, typename ...Args>
static void async(promise<T>* p, T(*handle)(Args...) ,Args... args) {
    return job_manager::enqueue_job<T, Args...>(p, handle, args...);
}

template<typename T, typename ...Args>
static void async(promise<T>* p, T(*handle)(Args...), std::tuple<Args...> args[], unsigned int count) {
    return job_manager::enqueue_jobs<T, Args...>(p, handle, args, count);
}

template<typename T, typename ...Args>
static void sync(promise<T>* p, T(*handle)(Args...), Args... args) {
    return job_manager::enqueue_job_and_wait(p, handle, args...);
}

template<typename T, typename ...Args>
static void sync(promise<T>* p, T(*handle)(Args...), std::tuple<Args...> args[], unsigned int count) {
    return job_manager::enqueue_jobs_and_wait(p, handle, args, count);
}

static void done() {
    worker::all_workers::done();
}

static void init(void(*handler)(void* data), void* data) {
    workers_count = thread::hardware_concurrency();
    workers = (worker**)malloc(sizeof(worker*)*workers_count - 1);

    for(int i=0; i<workers_count - 1; i++) {
        workers[i] = new worker();
    }

    worker::convert_thread_to_worker(handler, data);
}

static void wait_jobs() {
    for(int i=0; i<workers_count; i++) {
        workers[i]->wait();
    }
}


static void shut_down() {
    for(int i=0; i<workers_count - 1; i++) {
        delete workers[i];
    }  
    // delete workers;
}

};



// /*
//     Overload new operator.
// */
// void* operator new(size_t size){
//     return malloc(size);
// }

// /*
//     Overload delete operator.
// */
// void operator delete(void* ptr) {
//     return free(ptr);
// }


#endif