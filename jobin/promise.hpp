#ifndef JOBIN_PROMISE_H
#define JOBIN_PROMISE_H

#include <tuple>
#include <type_traits>



template <typename T> struct trait {
    typedef T type;
    T value = T();
    bool has_valid_value = true;

    trait() {}
    trait(T value): value {value} {}
    trait(trait<T>&& other): value {other.value} {}
};

template <> struct trait<void> {
    typedef void type;
    bool has_valid_value = false;

    trait() {}
    trait(trait<void>&& other) {}
};

template<typename T>
class promise {
    template<typename Ret, typename... Args>
    friend class promise_resolver;

    friend class job_manager;
    friend class job;

    bool is_resolved = false;
    trait<T> promise_value;
public:

    promise(): promise_value{} {}

    bool has_valid_return() {
        return promise_value.has_valid_value;
    }


    template <typename Ret = T>
    typename std::enable_if<!std::is_void<Ret>::value>::type
    set_value(Ret&& v) {
        promise_value.value = v;
    }


    template <typename Ret = T>
    typename std::enable_if<!std::is_void<Ret>::value, Ret>::type
    value() {
        return promise_value.value;
    }

};

#endif