#ifndef JOBIN_PROMISE_H
#define JOBIN_PROMISE_H

#include <tuple>
#include <type_traits>

template<typename T>
class promise {
    template<typename Ret, typename... Args>
    friend class promise_resolver;

    friend class job_manager;
    friend class job;

    bool is_resolved = false;

    T promise_value;
    
public:
    T value() {
        return promise_value;
    }
};

#endif