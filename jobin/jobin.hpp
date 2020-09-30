#ifndef JOBIN_SYSTEM_H
#define JOBIN_SYSTEM_H

#include "job_manager.hpp"
#include "worker.hpp"

namespace jobin {

namespace {
    worker** workers = nullptr;
    unsigned int workers_count = 0;
};

template<typename T, typename ...Args>
static promise<T>* async(T(*handle)(Args...) ,Args... args) {
    return job_manager::get_ptr()->enqueue_job<T, Args...>(handle, args...);
}

template<typename T, typename ...Args>
static promise<T>* async(T(*handle)(Args...), std::tuple<Args...> args[], unsigned int count) {
    return job_manager::get_ptr()->enqueue_jobs<T, Args...>(handle, args, count);
}

template<typename T, typename ...Args>
static promise<T>* sync(T(*handle)(Args...), Args... args) {
    return job_manager::get_ptr()->enqueue_and_wait_jobs(handle, std::tuple<Args...>(args...), 1);
}

template<typename T, typename ...Args>
static promise<T>* sync(T(*handle)(Args...), std::tuple<Args...> args[], unsigned int count) {
    return job_manager::get_ptr()->enqueue_and_wait_jobs(handle, args, count);
}

template<typename T>
static void wait(promise<T>* p) {
    return job_manager::get_ptr()->wait_promise(p);
}

static int done(int i) {
    worker::done();
    return i;
}

static void init(int(*handler)(void* data), void* data) {
    job_manager::init();
    workers_count = thread::hardware_concurrency();
    workers = (worker**)malloc(sizeof(worker*)*workers_count - 1);
        
    workers[0] = new worker(handler, data);
    for(int i=1; i<workers_count; i++) {
        workers[i] = new worker();
    }
}

static void wait_jobs() {
    for(int i=0; i<workers_count; i++) {
        workers[i]->wait();
    }
}

static void shut_down() {
    job_manager::shut_down();
    for(int i=1; i<workers_count; i++) {
        workers[i]->~worker();
    }
    delete workers;
}

};


#endif