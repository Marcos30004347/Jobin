#ifndef JOBIN_FIBER_H
#define JOBIN_FIBER_H

#include "config.hpp"
#include <stdlib.h>

#ifdef FIBER_FCONTEXT_BACKEND
#include "fcontext/fcontext.h"
#elif FIBER_WINDOWS_BACKEND
#include <windows.h>
#include <malloc.h>
#elif FIBER_EMSCRIPTEN_BACKEND
#include <malloc.h>
#include <emscripten.h>
#include <emscripten/fiber.h>
#elif FIBER_CROSS_BACKEND
#include "cross-fiber/cross-fiber.hpp"
#include "atomic.hpp"
using cross_fiber = fiber_t;
#endif



#ifdef FIBER_CROSS_BACKEND
static atomic<fiber_t*> primary {nullptr};
#endif

class fiber;

struct context_switch_data {
    fiber* to;
    fiber* from;
    context_switch_data(fiber* _from, fiber* _to): to {_to}, from {_from} {}
};

class fiber {
private:
    void(*handle)(void*);
    void* args;

    #ifdef FIBER_FCONTEXT_BACKEND

    fcontext_t ctx;
    fcontext_stack_t stack =  { nullptr, 0 };
    friend void fiber_entry(fcontext_transfer_t caller);
    friend void fiber_to_thread_entry(fcontext_transfer_t caller);

    #elif FIBER_WINDOWS_BACKEND
    
    void* stack;
    unsigned int stack_size;
    void* ctx;
    friend void fib_handle(void* fib);

    #elif FIBER_EMSCRIPTEN_BACKEND

    emscripten_fiber_t* ctx = nullptr;

    #elif FIBER_CROSS_BACKEND

    cross_fiber* ctx;

    #endif

    friend fiber* create_fiber(void(*handle)(void*), void* args);
    friend void init_fiber(fiber* fib, void(*handle)(void*), void* args);
    friend void convert_thread_to_fiber(fiber* fib,  void(*handle)(void*), void* arg);
    friend void switch_to_fiber(fiber* from, fiber* to);
    friend void destroy_fiber(fiber* fib);
    friend void reset_fiber(fiber* fib, void(*handle)(void*), void* args);
    friend void create_fiber_stack(fiber* fib, unsigned int stack_size);
    friend void destroy_fiber_stack(fiber* fib);

public:
    #ifdef FIBER_FCONTEXT_BACKEND
    fiber(): ctx{nullptr}, stack{ nullptr,0 }, handle{ nullptr }, args{ 0 } {}
    #elif FIBER_WINDOWS_BACKEND
    fiber(): ctx{nullptr}, stack{ nullptr }, handle{nullptr}, stack_size{0}, args{0} {}
    #elif FIBER_EMSCRIPTEN_BACKEND
    fiber(): stack{ nullptr }, handle{nullptr}, stack_size{0}, args{0},  allocator{nullptr} {}
    #elif FIBER_CROSS_BACKEND
    fiber(): ctx{nullptr}, handle{nullptr}, args{0}, allocator{nullptr} {}
    #endif
};

thread_local static fiber* current_fiber = nullptr;

#ifdef FIBER_FCONTEXT_BACKEND

void fiber_entry(fcontext_transfer_t caller) {
    context_switch_data* data = reinterpret_cast<context_switch_data*>(caller.data);
    data->from->ctx = caller.ctx;
    data->to->handle(data->to->args);
}

void fiber_to_thread_entry(fcontext_transfer_t caller) {
    fiber* fib = reinterpret_cast<fiber*>(caller.data);
    fib->ctx = caller.ctx;
    fib->handle(fib->args);

    jump_fcontext(caller.ctx, nullptr);
}

#elif FIBER_WINDOWS_BACKEND

void fib_handle(void* fib) {
    fiber* f = reinterpret_cast<fiber*>(fib);
    f->handle(f->args);
    exit(0);
}

#endif

void switch_to_fiber(fiber* from, fiber* to) {
    current_fiber = to;
    #ifdef FIBER_FCONTEXT_BACKEND

    context_switch_data* data = new context_switch_data(from, to);
    fcontext_transfer_t returner = jump_fcontext(to->ctx, data);
    data = reinterpret_cast<context_switch_data*>(returner.data);
    data->from->ctx = returner.ctx;

    #elif FIBER_WINDOWS_BACKEND

    SwitchToFiber(to->ctx);

    #elif FIBER_EMSCRIPTEN_BACKEND

    emscripten_fiber_swap(from->ctx, to->ctx);

    #elif FIBER_CROSS_BACKEND

    fiber_switch(to->ctx);

    #endif
    current_fiber = from;
}

void create_fiber_stack(fiber* fib, unsigned int stack_size = 1024 * 60) {
    #ifdef FIBER_FCONTEXT_BACKEND
    if(fib->stack.ssize) {
        destroy_fcontext_stack(&fib->stack);
    }
    fib->stack = create_fcontext_stack(stack_size);
    #elif FIBER_WINDOWS_BACKEND
    fib->stack = _malloca(stack_size);;
    fib->stack_size = stack_size;
    #elif FIBER_EMSCRIPTEN_BACKEND
    void* stack = malloc(stack_size);
    void* astack = malloc(stack_size);
    fib->astack = astack;
    fib->stack = stack;
    fib->stack_size = stack_size;
    #elif FIBER_CROSS_BACKEND
    // Stack managed by the fiber api
    #endif
}

void destroy_fiber_stack(fiber* fib) {
    #ifdef FIBER_FCONTEXT_BACKEND
    destroy_fcontext_stack(&fib->stack);
    #elif FIBER_WINDOWS_BACKEND
    delete fib->stack;
    #elif FIBER_EMSCRIPTEN_BACKEND
    delete fib->stack;
    delete fib->astack;
    #elif FIBER_CROSS_BACKEND
    // Stack managed by the fiber api
    #endif
}

void init_fiber(fiber* fib, void(*handle)(void*), void* args) {

    fib->args = args;
    fib->handle = handle;
    create_fiber_stack(fib);

    #ifdef FIBER_FCONTEXT_BACKEND
    fib->ctx = make_fcontext(fib->stack.sptr, fib->stack.ssize, fiber_entry);
    #elif FIBER_WINDOWS_BACKEND
    fib->ctx = CreateFiber(stack_s, stack, fib);
    #elif FIBER_EMSCRIPTEN_BACKEND
    emscripten_fiber_init(
        fib->ctx,
        fiber::callback,
        fib,
        stack,
        stack_s,
        astack,
        stack_s
    );
    #elif FIBER_CROSS_BACKEND
    fib->ctx = fiber_create(primary.load(), 1024*60, fib_handle, fib);
    #endif
}

fiber* create_fiber(void(*handle)(void*), void* args) {
    fiber* fib = new fiber;
    init_fiber(fib, handle, args);
    return fib;
}

void reset_fiber(fiber* fib, void(*handle)(void*), void* args) {
    fib->args = args;
    fib->handle = handle;

    #ifdef FIBER_FCONTEXT_BACKEND
    fib->ctx = make_fcontext(fib->stack.sptr, fib->stack.ssize, fiber_entry);
    #elif FIBER_WINDOWS_BACKEND
    if(fib->ctx) { DeleteFiber(fib->ctx); }
    fib->ctx = CreateFiber(fib->stack_size, fib->stack, fib);
    #elif FIBER_EMSCRIPTEN_BACKEND
    if(fib->stack) delete fib->stack;
    if(fib->astack) delete fib->astack;
    emscripten_fiber_init(
        fib->ctx,
        fiber::callback,
        fib,
        fib->stack,
        fib->stack_size,
        fib->astack,
        fib->stack_size
    );
    #elif FIBER_CROSS_BACKEND
    if(fib->ctx) fiber_delete(fib->ctx);
    fib->ctx = fiber_create(primary.load(), 1024*60, fib_handle, fib);
    #endif
}

void convert_thread_to_fiber(fiber* fib, void(*handle)(void*), void* arg) {
    fib->handle = handle;
    fib->args = arg;
    current_fiber = fib;

    create_fiber_stack(fib);

    #ifdef FIBER_FCONTEXT_BACKEND
    fib->ctx = make_fcontext(fib->stack.sptr, fib->stack.ssize, fiber_to_thread_entry);
    fib->ctx = jump_fcontext(fib->ctx, fib).ctx;
    #elif FIBER_WINDOWS_BACKEND
    ConvertThreadToFiber(nullptr);
    fib->ctx = GetCurrentFiber();
    fib->handle(fib->args);
    ConvertFiberToThread();
    #elif FIBER_EMSCRIPTEN_BACKEND
    emscripten_fiber_init_from_current_context(
        fib->ctx,
        astack,
        stack_s
    );
    fib->handle(args);
    #elif FIBER_CROSS_BACKEND
    primary.compare_exchange_strong(nullptr, fiber_create(0, 0, 0, nullptr));
    fib = create_fiber(init, args, fib_alocator);
    fib->allocator = &fib_alocator;
    fiber_switch(fib->ctx);
    #endif

    current_fiber = nullptr;
}

fiber* get_current_fiber() {
    return current_fiber;
}

void destroy_fiber(fiber* fib) {
    destroy_fiber_stack(fib);
    #ifdef FIBER_FCONTEXT_BACKEND
    // delete fib;
    #elif FIBER_WINDOWS_BACKEND
    DeleteFiber(fib->ctx);
    #elif FIBER_EMSCRIPTEN_BACKEND
    delete fib->ctx;
    // delete fib;
    #elif FIBER_CROSS_BACKEND
    fiber_delete(fib->ctx);
    // delete fib;
    #endif
}


#endif