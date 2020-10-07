#ifndef JOBIN_FIBER_POOL_fixed_H
#define JOBIN_FIBER_POOL_fixed_H

//Local libraries
#include "fiber.hpp"
#include <stdio.h>

template<unsigned int size = 256>
class avaliable_indices_stack {
private:
    unsigned int buff[size];
    unsigned int top;

public:
    avaliable_indices_stack(): top{0} { 
        for(int i=0; i<size; i++) make_avaliable(i);
    }

    ~avaliable_indices_stack() {} // delete buff;

    bool make_avaliable(unsigned int i) {
        if(top == size) {return false;}
        buff[top] = i;
        top++;
        return true;
    }

    bool get_avaliable(unsigned int &i) {
        if(top == 0) return false;
        top--;
        i = buff[top];
        return true;
    }   
};

template<unsigned int size = 256>
class fiber_pool_fixed {
    fiber buff[size];
    avaliable_indices_stack<size> avaliable_ids;

    public:

    fiber_pool_fixed();
    ~fiber_pool_fixed();

    bool release(fiber* &data);
    fiber* request();
};

template<unsigned int size>
fiber_pool_fixed<size>::fiber_pool_fixed(): avaliable_ids{} {}

template<unsigned int size>
fiber_pool_fixed<size>::~fiber_pool_fixed() {
    avaliable_ids.~avaliable_indices_stack();
}

template<unsigned int size>
bool fiber_pool_fixed<size>::release(fiber* &data) {
    if(!avaliable_ids.make_avaliable(data - buff)) {
        return false;
    }

    return true;
}
template<unsigned int size>
fiber* fiber_pool_fixed<size>::request() {
    unsigned int id;
    while(!avaliable_ids.get_avaliable(id)) {}
    return buff + id;
}


#endif