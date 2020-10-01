#ifndef JOBIN_ATOMIC_H
#define JOBIN_ATOMIC_H

#include <atomic>

template <typename T>
class atomic {
private:
    std::atomic<T> _atomic;

public:
    atomic(T initial): _atomic{initial} {}
    atomic(): _atomic{} {}
    ~atomic() {}

    bool compare_exchange_strong(T current_value, T new_value) { return _atomic.compare_exchange_strong(current_value, new_value); }
    bool compare_exchange_weak(T current_value, T new_value) { return _atomic.compare_exchange_weak(current_value, new_value); }
    T exchange(T value) { return _atomic.exchange(value); }
    T load() { return _atomic.load();}
    void store(T value) { _atomic.store(value);}

    atomic<T>& operator=(T&& value) {
        return atomic<T>(value);
    }

    // atomic<T> operator=(atomic<T> other) {
    //     return other;
    // }
};

#endif