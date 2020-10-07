#define TESTS

#include "jobin/worker.hpp"
#include "jobin/job_manager.hpp"

#include <assert.h>
#include <iostream>

#include "jobin/memory.hpp"

static bool worker_0 = false;
static bool worker_1 = false;

void worker_handler_0(void* data) {
    worker_0 = true;
    assert(data == nullptr);
    worker::all_workers::done();
}

void worker_handler_1(void* data) {
    assert(data == nullptr);
    worker_1 = true;
    worker::all_workers::done();
}

int main() {
    job_manager::init();
    worker::convert_thread_to_worker(worker_handler_0, nullptr);

    assert(worker_0 == true);

    worker worker1(worker_handler_1, nullptr);
    worker::all_workers::begin();
    worker1.wait();

    assert(worker_1 == true);

    worker1.~worker();
    job_manager::shut_down();
    assert(memory::currently_allocated_memory_count() == 0);
    return 0;
}