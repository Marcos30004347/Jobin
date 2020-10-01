/*
    @obs: If this file is included it will override the default new and delete operators.

    @opt: if RECORD_ALLOCATED_MEMORY_COUNT is defined, it will have one extra static method:
        * unsigned int currently_allocated_memory_count()
*/

#ifndef JOBIN_MEMORY_H
#define JOBIN_MEMORY_H

#include<malloc.h>

#ifdef RECORD_ALLOCATED_MEMORY_COUNT
#include"atomic.hpp"
#endif

#include<iostream>
class memory {
    #ifdef RECORD_ALLOCATED_MEMORY_COUNT
    // current bytes allocated count.
    static atomic<unsigned int> total_memory;
    #endif

public:

    /*
        Allocate @size bytes
        @param size - size in bytes to allocate.
    */
    static void* allocate(size_t size){

        #ifdef RECORD_ALLOCATED_MEMORY_COUNT
        size_t *p = (size_t*)malloc(size + sizeof(size_t));
        p[0] = size;
        total_memory.compare_exchange_strong(total_memory.load(), total_memory.load() + size);
        return (void*)(&p[1]);
        #else
        return malloc(size);
        #endif

    }

    /*
        Deallocate ptr memory
        @param ptr - memory to be deallocated.
    */
    static void deallocate(void* ptr) {
        #ifdef RECORD_ALLOCATED_MEMORY_COUNT

        size_t *p = (size_t*)ptr;
        size_t size = p[-1];
        total_memory.compare_exchange_strong(total_memory.load(), total_memory.load() - size);
        void *p2 = (void*)(&p[-1]);
        free(p2);
        #else
        free(ptr);
        #endif
    }

    #ifdef RECORD_ALLOCATED_MEMORY_COUNT
    /*
        Return current allocated bytes count.
    */
    static unsigned int currently_allocated_memory_count() {
        return total_memory.load();
    }
    #endif
};

#ifdef RECORD_ALLOCATED_MEMORY_COUNT
atomic<unsigned int> memory::total_memory{0};
#endif

/*
    Overload new operator.
*/
void* operator new(size_t size){
    return memory::allocate(size);
}

/*
    Overload delete operator.
*/
void operator delete(void* ptr) {
    return memory::deallocate(ptr);
}


#endif