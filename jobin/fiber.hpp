#ifndef JOBIN_FIBER_H
#define JOBIN_FIBER_H

#include "config.hpp"

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

#ifdef FIBER_FCONTEXT_BACKEND
struct context_switch_data {
    fiber* to = nullptr;
    fiber* from = nullptr;
    context_switch_data(fiber* _from, fiber* _to): to {_to}, from {_from} {}
};
#endif

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

    template<unsigned int size>
    friend class fiber_pool_fixed;

    friend fiber* create_fiber();
    friend void init_fiber(fiber* fib, void(*handle)(void*), void* args);
    friend void convert_thread_to_fiber(fiber* fib,  void(*handle)(void*), void* arg);
    friend void switch_to_fiber(fiber* from, fiber* to);
    friend void destroy_fiber(fiber* fib);
    friend void reset_fiber(fiber* fib, void(*handle)(void*), void* args);
    friend void create_fiber_stack(fiber* fib, unsigned int stack_size);
    friend void destroy_fiber_stack(fiber* fib);

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

/**
 * Saves current context of @from and switch to @to context of execution.
 * @param from: fiber to save the context.
 * @param to: fiber to switch to.
 */
void switch_to_fiber(fiber* from, fiber* to);

void create_fiber_stack(fiber* fib, unsigned int stack_size = 1024 * 60);

void destroy_fiber_stack(fiber* fib);

void init_fiber(fiber* fib, void(*handle)(void*), void* args);

fiber* create_fiber();

void reset_fiber(fiber* fib, void(*handle)(void*), void* args);

void convert_thread_to_fiber(fiber* fib, void(*handle)(void*), void* arg);

fiber* get_current_fiber();

void destroy_fiber(fiber* fib);


#endif