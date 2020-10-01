#include "jobin/thread.hpp"

#include <assert.h>
#include <iostream>

#define RECORD_ALLOCATED_MEMORY_COUNT
#include "jobin/memory.hpp"

bool thread_executed = false;

void thread_handle(void*) {
    for(int i=0; i<thread::hardware_concurrency(); i++) {
        assert(thread::this_thread::set_affinity(0));
    }

    thread_executed = true;
}

int main() {
    thread t(thread_handle, nullptr);
    t.join();

    assert(thread_executed == true);
    assert(thread::hardware_concurrency());
    assert(memory::currently_allocated_memory_count() == 0);
    return 0;
}