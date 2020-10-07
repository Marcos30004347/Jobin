#include "jobin/job_manager.hpp"
#include "jobin/worker.hpp"

#include <assert.h>
#include <iostream>

#define TESTS
#include "jobin/memory.hpp"

int handle(int a, int b) {
    return a+b;
}

void worker_handler(void* data) {

    promise<int>* p = job_manager::get_ptr()->enqueue_job(handle, 1, 2);
    assert(p->is_resolved == false);
    p->wait();
    assert(p->is_resolved == true);
    assert(p->value() == 3);
    delete p;

    p = job_manager::get_ptr()->enqueue_job_and_wait(handle, 1, 2);
    assert(p->value() == 3);
    delete p;

    std::tuple<int, int> args[2];
    args[0] = {1, 2};
    args[1] = {2, 3};

    promise<int>* ps = job_manager::get_ptr()->enqueue_jobs(handle, args, 2);
    ps[0].wait();
    ps[1].wait();

    assert(ps[0].value() == 3);
    assert(ps[1].value() == 5);
    delete ps;

    ps = job_manager::get_ptr()->enqueue_jobs_and_wait(handle, args, 2);
    assert(ps[0].value() == 3);
    assert(ps[1].value() == 5);

    delete ps;

    worker::all_workers::done();
}

int main() {
    job_queues queues;

    promise<int> k;
    job* j = new job(&k, handle, 1, 2);
    job* t = nullptr;

    assert(queues.enqueue(j));
    assert(queues.dequeue(t));
    assert(t == j);

    delete j;
    assert(memory::currently_allocated_memory_count() == 0);
    assert(job_manager::get_ptr() == nullptr);
    job_manager::init();
    assert(job_manager::get_ptr());

    worker::convert_thread_to_worker(worker_handler, nullptr);

    job_manager::shut_down();

    assert(memory::currently_allocated_memory_count() == 0);

    return 0;
}