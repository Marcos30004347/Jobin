#ifndef JOBIN_VECTOR_H
#define JOBIN_VECTOR_H

#include "spin_lock.hpp"
#include <string.h>
#include <stdio.h>

template <typename T>
class buffer_iterator;


template <typename T>
class vector {
private:

    T* buff;
    spin_lock mutex;
    unsigned int length;
    unsigned int reserved;

    void increase_buffer(unsigned int s);
    void decrease_buffer(unsigned int s);

public:
    using iterator = buffer_iterator<T>;
    iterator& begin();
    iterator& end();
    vector(unsigned int reserve = 32);
    ~vector();
    unsigned int size();
    void push_back(const T& value);
    void push_front(const T& value);
    void pop_front();
    void pop_back();

    T& operator[](unsigned int index);
};



template <typename T>
class buffer_iterator {
private:
    T* buff;

public:
    buffer_iterator(T* b): buff{b} {}
    ~buffer_iterator() {}

    buffer_iterator<T>& operator++() { buff++; return *this; }
    buffer_iterator<T> operator++(int) { buffer_iterator<T> tmp(*this); operator++(); return tmp; }
    
    buffer_iterator<T>& operator--() { buff--; return *this; }
    buffer_iterator<T> operator--(int) { buffer_iterator<T> tmp(*this); operator--(); return tmp; }

    buffer_iterator<T>& operator +(unsigned int v) { buff+=v; return *this; }
    buffer_iterator<T>& operator -(unsigned int v) { buff-=v; return *this; }

    bool operator != (T v) {return v != *buff; }
    bool operator != (T* v) {return v != buff; }
    bool operator != (const buffer_iterator<T>& it) {return it.buff != buff; }

    bool operator <= (T v) {return v <= *buff; }
    bool operator <= (T* v) {return v <= buff; }
    bool operator <= (const buffer_iterator<T>& it) {return it.buff <= buff; }

    bool operator >= (T v) {return v >= *buff; }
    bool operator >= (T* v) {return v >= buff; }
    bool operator >= (const buffer_iterator<T>& it) {return it.buff >= buff; }

    bool operator < (T v) {return v < *buff; }
    bool operator < (T* v) {return v < buff; }
    bool operator < (const buffer_iterator<T>& it) {return it.buff < buff; }

    bool operator > (T v) {return v > *buff; }
    bool operator > (T* v) {return v > buff; }
    bool operator > (const buffer_iterator<T>& it) {return it.buff > buff; }

    buffer_iterator<T> operator = (const buffer_iterator<T>& it) {buff = it.buff; }
    T* value() { return buff; }
};



template <typename T>
void vector<T>::increase_buffer(unsigned int s) {
    reserved += s;
    T* old_buff = buff;
    buff = new T[length + s];
    memcpy(buff, old_buff, length * sizeof(T));
    delete old_buff;
}

template <typename T>
void vector<T>::decrease_buffer(unsigned int s) {
    reserved -= s;
    T* old_buff = buff;
    buff = new T[length - s];
    memcpy(buff, old_buff, (length - s) * sizeof(T));        
    delete old_buff;
}

template <typename T>
buffer_iterator<T>& vector<T>::begin() {
    return iterator(buff);
}

template <typename T>
buffer_iterator<T>& vector<T>::end() {
    return iterator(buff + length);
}


template <typename T>
vector<T>::vector(unsigned int reserve): mutex{} {
    reserved = reserve;
    buff = new T[reserved];
}

template <typename T>
vector<T>::~vector() { mutex.lock(); if(buff) delete buff; mutex.unlock(); }

template <typename T>
unsigned int vector<T>::size() {
    return length;
}

template <typename T>
void vector<T>::push_back(const T& value) {
    mutex.lock();

    if(length >= reserved) {
        increase_buffer(length/2);
    }

    buff[length] = value;
    length++;

    mutex.unlock();
}

template <typename T>
void vector<T>::push_front(const T& value) {
    mutex.lock();

    if(length >= reserved) {
        increase_buffer(length/2);
    }

    memcpy(buff+1, buff, (length * sizeof(T)));
    
    buff[0] = value;
    length++;
    mutex.unlock();
}

    
template <typename T>
void vector<T>::pop_front() {

    mutex.lock();

    if(length <= 0) {
        length = 0;
        mutex.unlock();
        return; 
    }

    T* old_buff = buff;
    buff = new T[length - 1];
    memcpy(buff, old_buff + 1, (length - 1) * sizeof(T));  
    length--;      
    delete old_buff;

    mutex.unlock();
}

template <typename T>
void vector<T>::pop_back() {

    mutex.lock();
    if(length <= 0) {
        length = 0;
        mutex.unlock();
        return; 
    }

    length--;
    mutex.unlock();
}

template <typename T>
T& vector<T>::operator[](unsigned int index) {
    return buff[index];
}



#endif