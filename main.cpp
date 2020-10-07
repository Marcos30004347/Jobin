#include "jobin.hpp"

using namespace jobin;

int print_numbers() {
    printf("1\n");
    printf("2\n");
    printf("3\n");
    printf("4\n");
    printf("5\n");
    printf("6\n");
    printf("7\n");
    printf("8\n");
    printf("9\n");
    printf("10\n");
    printf("11\n");
    printf("12\n");
    printf("13\n");
    return 0;
}

void init_system(void* data) {
    promise<int>** p3 = new promise<int>*[10];

    for(int i=0; i<10; i++) {
        p3[i] = async(print_numbers);
    }

    for(int i=0; i<10; i++) {
        wait(p3[i]);
    }

    done();
}


int main() {
    init(init_system, nullptr);
    wait_jobs();
    shut_down();
}