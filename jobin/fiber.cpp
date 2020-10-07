#include <stdlib.h>

#include "fiber.hpp"

#ifdef FIBER_FCONTEXT_BACKEND

void fiber_entry(fcontext_transfer_t caller) {
    context_switch_data* data = reinterpret_cast<context_switch_data*>(caller.data);
    data->from->ctx = caller.ctx;

    void(*handle)(void*) = data->to->handle;
    void* args = data->to->args;
    delete data;
    handle(args);
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

/**
 * Saves current context of @from and switch to @to context of execution.
 * @param from: fiber to save the context.
 * @param to: fiber to switch to.
 */
void switch_to_fiber(fiber* from, fiber* to) {
    current_fiber = to;

    #ifdef FIBER_FCONTEXT_BACKEND
    context_switch_data* data = new context_switch_data(from, to);
    fcontext_transfer_t returner = jump_fcontext(to->ctx, data);
    data = reinterpret_cast<context_switch_data*>(returner.data);
    data->from->ctx = returner.ctx;
    delete data;

    #elif FIBER_WINDOWS_BACKEND

    SwitchToFiber(to->ctx);

    #elif FIBER_EMSCRIPTEN_BACKEND

    emscripten_fiber_swap(from->ctx, to->ctx);

    #elif FIBER_CROSS_BACKEND

    fiber_switch(to->ctx);

    #endif
    current_fiber = from;
}

void create_fiber_stack(fiber* fib, unsigned int stack_size) {
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
    if(!current_fiber) abort();
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

fiber* create_fiber() {
    fiber* fib = new fiber();
    return fib;
}

void reset_fiber(fiber* fib, void(*handle)(void*), void* args) {
    if(!current_fiber) abort();

    #ifdef FIBER_FCONTEXT_BACKEND
    
    if(!fib->stack.ssize) create_fiber_stack(fib);
    fib->ctx = make_fcontext(fib->stack.sptr, fib->stack.ssize, fiber_entry);

    #elif FIBER_WINDOWS_BACKEND
    if(fib->ctx) { DeleteFiber(fib->ctx); }
    fib->ctx = CreateFiber(fib->stack_size, fib->stack, fib);
    #elif FIBER_EMSCRIPTEN_BACKEND
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

    fib->args = args;
    fib->handle = handle;
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
    delete fib;
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
