#include"pool_allocator.hpp"


memory_chunk* pool_allocator::allocate_block(unsigned int n_blocks) {
    unsigned int size = block_size * n_blocks;

    memory_chunk* block = reinterpret_cast<memory_chunk*>(malloc(size));


    last_block->next = reinterpret_cast<memory_block*>(block);
    last_block->next->next = nullptr;

    last_block = last_block->next;

    memory_chunk* chunk = block;

    for(int i=1; i<n_blocks; i++) {
        chunk->next = reinterpret_cast<memory_chunk*>(reinterpret_cast<char*>(chunk) + block_size);
        chunk = chunk->next;
    }

    return block;
}

pool_allocator::pool_allocator(unsigned int block_size, unsigned int pre_alocated_blocks): block_size{block_size} {
    first_block = new memory_block;
    first_block->next = nullptr;
    last_block = first_block;

    allocate_block(pre_alocated_blocks);
}

pool_allocator::~pool_allocator() {
    memory_block* f = first_block->next;
    while (f)
    {
        f = first_block->next;
        delete first_block;
        first_block = f;
    }
}

void* pool_allocator::alloc() {
    allocations++;
    if(!mAlloc) {
        mAlloc = allocate_block(10);
    }

    memory_chunk* free_chunk = mAlloc;
    mAlloc = mAlloc->next;

    return free_chunk;
}

void pool_allocator::free(void* memory) {
    dealocations++;
    reinterpret_cast<memory_chunk*>(memory)->next = mAlloc;
    mAlloc = reinterpret_cast<memory_chunk*>(memory);

}