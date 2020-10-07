#ifndef JOBIN_POOL_ALLOCATOR_H
#define JOBIN_POOL_ALLOCATOR_H

#include <malloc.h>
#include "spin_lock.hpp"

struct memory_chunk {
    memory_chunk* next = nullptr;
};

struct memory_block {
    void* ptr;
    memory_block* next;
};

class pool_allocator {
    memory_chunk* mAlloc = nullptr;

    memory_block* first_block = nullptr;
    memory_block* last_block = nullptr;

    unsigned int block_size;
    unsigned int allocations = 0;
    unsigned int dealocations = 0;

    memory_chunk* allocate_block(unsigned int n_blocks);
public:
    pool_allocator(unsigned int block_size, unsigned int pre_alocated_blocks = 0);
    ~pool_allocator();
    void* alloc();
    void free(void* memory);
};


#endif