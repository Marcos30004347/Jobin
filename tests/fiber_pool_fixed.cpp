#include "jobin/fiber_pool_fixed.hpp"

#include <assert.h>
#include <iostream>

#define RECORD_ALLOCATED_MEMORY_COUNT
#include "jobin/memory.hpp"


fiber_pool_fixed<256> pool;
fiber* thread_fiber;
fiber* context_fiber;

void fiber_handler(void* data) {
    switch_to_fiber(get_current_fiber(), thread_fiber);
}

void initial_fiber_handler(void* data) {
    context_fiber = pool.request();
    reset_fiber(context_fiber, fiber_handler, nullptr);
    switch_to_fiber(thread_fiber, context_fiber);
    pool.release(context_fiber);
}

int main() {
    thread_fiber = pool.request();
    convert_thread_to_fiber(thread_fiber, initial_fiber_handler, nullptr);
    pool.release(thread_fiber);

    assert(memory::currently_allocated_memory_count() == 0);
    return 0;
}