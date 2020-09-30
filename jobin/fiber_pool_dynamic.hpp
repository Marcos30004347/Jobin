#ifndef JOBIN_FIBEr_POOL_dynamic_H
#define JOBIN_FIBEr_POOL_dynamic_H

//Local libraries
#include "fiber.hpp"
#include "pool_allocator.hpp"


class fiber_pool_dynamic {
    pool_allocator allocator;

    public:

    fiber_pool_dynamic();
    ~fiber_pool_dynamic();

    bool release(fiber* &data);
    bool request(fiber* &value, void(*handle)(void*), void* args);
};

fiber_pool_dynamic::fiber_pool_dynamic(): allocator{sizeof(fiber), 256} {}

fiber_pool_dynamic::~fiber_pool_dynamic() {}

bool fiber_pool_dynamic::release(fiber* &data) {
    destroy_fiber_stack(data);
    allocator.free(data);
    return true;
}

bool fiber_pool_dynamic::request(fiber* &value, void(*handle)(void*), void* args) {
    value = (fiber*)allocator.alloc();

    create_fiber_stack(value);
    reset_fiber(value, handle, args);

    return true;
}


#endif