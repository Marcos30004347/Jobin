#define TESTS
#include "jobin/job.hpp"
#include "jobin/memory.hpp"

#include <assert.h>
#include <iostream>

int sum(int a, int b) {
    return a + b;
}


int main() {
    promise<int> promise;
    job j(&promise, sum, 1, 2);

    // assert(memory::currently_allocated_memory_count() == 0);
    return 0;
}