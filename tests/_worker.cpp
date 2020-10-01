#include "jobin/worker.hpp"

#include <assert.h>
#include <iostream>

#define RECORD_ALLOCATED_MEMORY_COUNT
#include "jobin/memory.hpp"

void worker_handler(void* data) {
    worker::all_workers::done();
}

int main() {
    worker worker0(worker_handler, nullptr);
    worker0.wait();
    worker0.~worker();

    assert(memory::currently_allocated_memory_count() == 0);
    return 0;
}