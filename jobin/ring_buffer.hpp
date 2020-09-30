#ifndef JOBIN_RING_BUFFER_H
#define JOBIN_RING_BUFFER_H

#include "spin_lock.hpp"
// #include<mutex>

template<typename T, unsigned int size>
class ring_buffer {

    T* buff;
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
    buff = new T[size+1];
}

template<typename T, unsigned int size>
ring_buffer<T,size>::~ring_buffer() {
    mutex.lock();
    if(buff) delete buff;
    mutex.unlock();
}

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