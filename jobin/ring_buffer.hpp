#ifndef JOBIN_RING_BUFFER_H
#define JOBIN_RING_BUFFER_H

#include "spin_lock.hpp"
// #include<mutex>
#include<iostream>

template<typename T, unsigned int size>
class ring_buffer {

    T buff[size];
    spin_lock mutex;
    // std::mutex mutex;

    unsigned int top,tail;
    public:

    ring_buffer();
    ~ring_buffer();
    bool enqueue(const T& data);
    bool dequeue(T& value);

};

template<typename T, unsigned int size>
ring_buffer<T,size>::ring_buffer(): mutex{} {
    // std::cout << "creating ring buffer" << std::endl;
}

template<typename T, unsigned int size>
ring_buffer<T,size>::~ring_buffer() {}

template<typename T, unsigned int size>
bool ring_buffer<T,size>::enqueue(const T& data) {
    mutex.lock();
    if((top + 1)%(size+1) == tail) {
        mutex.unlock();
        return false;
    }
    buff[top] = data;
    top = (top + 1)%(size+1);
    mutex.unlock();
    return true;
}

#include<stdio.h>

template<typename T, unsigned int size>
bool ring_buffer<T,size>::dequeue(T& value) {
    mutex.lock();
    if(top == tail) {
        mutex.unlock();
        return false;
    }

    value = buff[tail];
    tail = (tail + 1)%(size+1);

    mutex.unlock();
    return true;
}



#endif