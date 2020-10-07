#include "jobin/fiber.hpp"

#define TESTS
#include "jobin/memory.hpp"

#include <assert.h>

fiber* thread_fiber = nullptr;
fiber* context_fiber = nullptr;

bool was_fiber_handle0_executed = false;
bool was_fiber_handle1_executed = false;

void fiber_handle0(void* data) {
    assert(data == nullptr);
    assert(get_current_fiber() == context_fiber);

    was_fiber_handle0_executed = true;
    switch_to_fiber(context_fiber, thread_fiber);
}

void fiber_handle1(void* data) {
    assert(data == nullptr);
    assert(get_current_fiber() == context_fiber);

    was_fiber_handle1_executed = true;
    switch_to_fiber(context_fiber, thread_fiber);
}

void init_thread(void* data) {
    assert(get_current_fiber() == thread_fiber);

    context_fiber = create_fiber();
    init_fiber(context_fiber, fiber_handle0, nullptr);

    switch_to_fiber(thread_fiber, context_fiber);

    reset_fiber(context_fiber, fiber_handle1, nullptr);

    switch_to_fiber(thread_fiber, context_fiber);

    destroy_fiber(context_fiber);
    return;
}


int main() {
    thread_fiber = create_fiber();
    convert_thread_to_fiber(thread_fiber, init_thread, nullptr);
    destroy_fiber(thread_fiber);

    // assert that handle 0 was executed
    assert(was_fiber_handle0_executed);

    // assert that handle 1 was executed
    assert(was_fiber_handle1_executed);

    // assert that all allocated memory was correctly deallocated
    assert(memory::currently_allocated_memory_count() == 0);

    return 0;
}