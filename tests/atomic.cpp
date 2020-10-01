#include "jobin/atomic.hpp"

#include <assert.h>
#include <iostream>

#define RECORD_ALLOCATED_MEMORY_COUNT
#include "jobin/memory.hpp"

int main() {
    atomic<int> a {4};
    assert(a.load() == 4);
    assert(a.compare_exchange_strong(4, 5));
    assert(a.load() == 5);
    assert(a.compare_exchange_strong(5, 6));
    assert(a.load() == 6);
    a.exchange(7);
    assert(a.load() == 7);
    a.store(8);
    assert(a.load() == 8);

    assert(memory::currently_allocated_memory_count() == 0);
    return 0;
}