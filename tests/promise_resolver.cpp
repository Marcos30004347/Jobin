#include "jobin/promise_resolver.hpp"

#define TESTS
#include "jobin/memory.hpp"

#include <assert.h>
#include <iostream>

bool void_executed = false;

int sum(int& a, int& b) {
    int c = a + b;
    a = 0;
    b = 0;
    return c;
}

void void_fn() {
    void_executed = true;
    return;
}


void void_fn_char_prt(char* data) {
    *data = 'c';
    return;
}



int main() {
    char* character = new char;

    promise<int> promise0;
    promise<void> promise1;
    promise<void> promise2;
    int a=1, b=2;
    promise_resolver<int, int&, int&> resolver_sum(sum, a, b);
    
    promise_resolver<void> resolver_void(void_fn);

    promise_resolver<void, char*> resolver_char(void_fn_char_prt, character);
    

    promise_resolver<int, int&, int&>::resolve_with_resolver(&promise0, &resolver_sum);
    promise_resolver<void>::resolve_with_resolver(&promise1, &resolver_void);
    promise_resolver<void, char*>::resolve_with_resolver(&promise2, &resolver_char);

    assert(*character == 'c');
    assert(void_executed == true);

    assert(promise0.value() == 3);
    assert(a == 0);
    assert(b == 0);

    delete character;

    assert(memory::currently_allocated_memory_count() == 0);

    return 0;
}