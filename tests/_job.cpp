#include "jobin/job.hpp"

#include <assert.h>
#include <iostream>

#define RECORD_ALLOCATED_MEMORY_COUNT
#include "jobin/memory.hpp"

int sum(int a, int b) {
    return a + b;
}

int main() {
    promise<int> promise;
    job j(&promise, sum, 1, 2);
    job::execute(&j);
    assert(promise.value() == 3);

    assert(memory::currently_allocated_memory_count() == 0);
    return 0;
}