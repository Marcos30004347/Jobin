#include "jobin/promise_resolver.hpp"

#define RECORD_ALLOCATED_MEMORY_COUNT
#include "jobin/memory.hpp"

#include <assert.h>
#include <iostream>

bool void_executed = false;

int sum(int a, int b) {
    return a+b;
}

void void_fn() {
    void_executed = true;
    return;
}

int main() {

    promise<int> promise0;
    promise<int> promise1;
    promise<void> promise2;
    promise_resolver<int, int, int> resolver(sum);
    promise_resolver<void> resolver_void(void_fn);

    resolver.set_args(1, 2);

    promise_resolver<int, int, int>::resolve_with_resolver(&promise0, &resolver);
    promise_resolver<void>::resolve_with_resolver(&promise2, &resolver_void);

    resolver.resolve(&promise1);

    assert(void_executed == true);
    assert(promise0.value() == 3);
    assert(promise1.value() == 3);
    assert(memory::currently_allocated_memory_count() == 0);

    return 0;
}