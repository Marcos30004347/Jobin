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

#endif