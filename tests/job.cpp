#define TESTS
#include "jobin/job.hpp"
#include "jobin/worker.hpp"
#include "jobin/memory.hpp"

#include <assert.h>
#include <iostream>

int sum(int a, int b) {
    return a + b;
}

void worker_handler(void* data) {
    promise<int> promise;
    job j(&promise, sum, 1, 2);

    job::execute(&j);
    assert(promise.value() == 3);
}

int main() {
    worker::convert_thread_to_worker(worker_handler, nullptr);
    assert(memory::currently_allocated_memory_count() == 0);
    return 0;
}