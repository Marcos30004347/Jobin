#ifndef JOBIN_MUTEX_H
#define JOBIN_MUTEX_H
#include <mutex>

class mutex {
    private:
    std::mutex mut;
    public:

    mutex();
    ~mutex();

    void lock();
    void unlock();
};


mutex::mutex() {}
mutex::~mutex() { mut.~mutex(); }

void mutex::lock() { mut.lock(); }
void mutex::unlock() { mut.unlock(); }


#endif