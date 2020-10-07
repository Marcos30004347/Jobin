#include "jobin/promise.hpp"

#define TESTS
#include "jobin/memory.hpp"

#include <assert.h>
#include <iostream>



int main() {

    promise<int> promise0;
    promise0.set_value(3);

    promise<void> promise1;
    assert(promise0.value() == 3);
    assert(promise0.has_valid_return() == true);
    assert(promise1.has_valid_return() == false);
    assert(memory::currently_allocated_memory_count() == 0);

    return 0;
}