#ifndef JOBIN_SPIN_LOCK_H
#define JOBIN_SPIN_LOCK_H

#include "atomic.hpp"
#include<stdio.h>
class spin_lock {
    private:
    atomic<int> _lock;

    public:
    spin_lock(): _lock{0} {}
    ~spin_lock() { _lock.~atomic(); }
    void lock();
    void unlock();
};

/**
 * exchenge chagens the value in the lock and return
 * the old value atomically, so if the lock was zero,
 * the loop wont be executed and the code continues,
 * if another thread try to lock they will be locked 
 * inside the loop, when Unlock runs, it will set lock
 * to zero, and one thread will be allowed to achire
 * the lock. 
 **/

void spin_lock::lock() { while(_lock.exchange(1) == 1) {} }
void spin_lock::unlock() { _lock.exchange(0); }


#endif