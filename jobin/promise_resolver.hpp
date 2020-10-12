#ifndef JOBIN_PROMISE_RESOLVER_H
#define JOBIN_PROMISE_RESOLVER_H

#include "promise.hpp"

/*
template params:
    Ret: return type
    Args: list of function argument types
*/


template<typename Ret, typename... Args>
class promise_resolver {
private:
    std::tuple<Args...> arguments;
    Ret (*handle)(Args...);

    template<std::size_t... Is>
    inline Ret invoke(const std::tuple<Args...>&& tuple, std::index_sequence<Is...>) noexcept {
        return std::forward<Ret>(handle(std::get<Is>(tuple)...));
    }

public:
    promise_resolver(Ret (*handle)(Args...), Args ...args) noexcept: handle{handle}, arguments{args...} {}
    promise_resolver(Ret (*handle)(Args...), std::tuple<Args...>&& args) noexcept: handle{handle}, arguments{args} {}

    static void resolve_with_resolver(promise<Ret>* p, promise_resolver<Ret, Args...>* inv) noexcept {
        p->set_value(std::forward<Ret>(inv->invoke(std::forward<std::tuple<Args...>>(inv->arguments), std::index_sequence_for<Args...>{})));
        p->is_resolved = true;
    }

};


template<typename... Args>
class promise_resolver<void, Args...> {
    friend class job;
private:
    std::tuple<Args...> arguments;
    void(*handle)(Args...);

    template<std::size_t... Is>
    inline void invoke(const std::tuple<Args...>& tuple, std::index_sequence<Is...>) noexcept {
        handle(std::get<Is>(tuple)...);
    }

public:
    promise_resolver(void(*handle)(Args...), Args... args) noexcept: handle{handle}, arguments{args...} {}
    promise_resolver(void (*handle)(Args...), std::tuple<Args...>&& args) noexcept: handle{handle}, arguments{args} {}

    // inline void set_args(Args... values) noexcept {
    //     arguments = std::tuple<Args...>(std::forward<Args>(values)...);
    // }

    // inline void set_args(std::tuple<Args...>&& args) noexcept {
    //     arguments = args;
    // }

    inline static void resolve_with_resolver(promise<void>* p, promise_resolver<void, Args...>* inv) noexcept {
        inv->invoke(inv->arguments, std::index_sequence_for<Args...>{});
        p->is_resolved = true;
    }


};



#endif