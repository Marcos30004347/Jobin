#define TESTS
#include "jobin/jobin.hpp"
#include "jobin/memory.hpp"

#include <assert.h>
#include <iostream>

int sum(int a, int b) {
    return a + b;
}

void init_system(void* data) {
    promise<int> p;
    jobin::async(&p, sum, 1, 2);
    p.wait();
    assert(p.value() == sum(1,2));

    promise<int> ps[2];
    std::tuple<int, int> args[2];
    args[0] = {1,2};
    args[1] = {2,3};

    jobin::async(ps, sum, args, 2);
    ps[0].wait();
    ps[1].wait();

    assert(ps[0].value() == sum(1,2));
    assert(ps[1].value() == sum(2,3));

    jobin::sync(&p, sum, 1, 2);
    assert(p.value() == sum(1,2));

    jobin::sync(ps, sum, args, 2);
    assert(ps[0].value() == sum(1,2));
    assert(ps[1].value() == sum(2,3));

    jobin::done();
}

int main() {
    jobin::init(init_system, nullptr);
    jobin::shut_down();
    assert(memory::currently_allocated_memory_count() == 0);
    return 0;
}