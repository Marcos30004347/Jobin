#ifndef JOBIN_RING_BUFFER_H
#define JOBIN_RING_BUFFER_H

#include "spin_lock.hpp"

template<typename T, unsigned int array_size>
class ring_buffer {

    T buff[array_size];
    spin_lock mutex;

    unsigned int top = 0, tail = 0, _size = 0;

    public:

    ring_buffer();
    ~ring_buffer();
    bool enqueue(const T& data);
    bool dequeue(T& value);

    unsigned int size() { return _size; }
};

template<typename T, unsigned int array_size>
ring_buffer<T,array_size>::ring_buffer(): mutex{}, top{0}, tail{0}, _size{0} {}

template<typename T, unsigned int array_size>
ring_buffer<T,array_size>::~ring_buffer() {}

template<typename T, unsigned int array_size>
bool ring_buffer<T,array_size>::enqueue(const T& data) {
    mutex.lock();
    if((top + 1)%(array_size+1) == tail) {
        mutex.unlock();
        return false;
    }
    _size++;
    buff[top] = data;

    top = (top + 1)%(array_size+1);

    mutex.unlock();
    return true;
}


template<typename T, unsigned int array_size>
bool ring_buffer<T,array_size>::dequeue(T& value) {
    mutex.lock();
    if(top == tail) {
        mutex.unlock();
        return false;
    }
    _size--;

    value = buff[tail];
    tail = (tail + 1)%(array_size+1);
    mutex.unlock();
    return true;
}



#endif