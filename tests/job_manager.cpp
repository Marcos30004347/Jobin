#define TESTS
#include "jobin/memory.hpp"
#include "jobin/worker.hpp"
#include "jobin/job_manager.hpp"

#include <assert.h>
#include <iostream>


int handle(int a, int b) {
    return a+b;
}

void worker_handler(void* data) {


    promise<int> p;
    

    job_manager::enqueue_job(&p, handle, 1, 2);

    assert(p.is_resolved == false);
    p.wait();
    assert(p.is_resolved == true);
    assert(p.value() == 3);


    job_manager::enqueue_job_and_wait(&p, handle, 1, 2);

    assert(p.value() == 3);


    std::tuple<int, int> args[2];
    promise<int> ps[2]; 

    args[0] = {1, 2};
    args[1] = {2, 3};

    job_manager::enqueue_jobs(ps, handle, args, 2);
    ps[0].wait();
    ps[1].wait();

    assert(ps[0].value() == 3);
    assert(ps[1].value() == 5);

    job_manager::enqueue_jobs_and_wait(ps, handle, args, 2);
    assert(ps[0].value() == 3);
    assert(ps[1].value() == 5);


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


    worker::convert_thread_to_worker(worker_handler, nullptr);

    assert(memory::currently_allocated_memory_count() == 0); 
    return 0;
}