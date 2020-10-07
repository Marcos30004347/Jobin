#define TESTS
#include "jobin/memory.hpp"
#include "jobin/ring_buffer.hpp"

#include <assert.h>
#include <iostream>

#define ring_buffer_size 40

int main() {
    ring_buffer<int, ring_buffer_size> rb;

    for(int round = 0; round< 20; round++) {
        for(int i=0; i<ring_buffer_size; i++) {
            assert(rb.size() == i);
            rb.enqueue(i);
            assert(rb.size() == i+1);

        }

        int t;
        for(int i=0; i<ring_buffer_size; i++) {
            assert(rb.size() == ring_buffer_size - i);
            rb.dequeue(t);
            assert(rb.size() == ring_buffer_size - i - 1);
        }
    }

    assert(memory::currently_allocated_memory_count() == 0);
    return 0;
}