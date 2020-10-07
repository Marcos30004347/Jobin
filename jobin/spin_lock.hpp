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



#endif