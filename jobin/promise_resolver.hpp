#ifndef JOBIN_PROMISE_RESOLVER_H
#define JOBIN_PROMISE_RESOLVER_H

#include <tuple>
#include "atomic.hpp"
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
    Ret(*handle)(Args...);

    template<std::size_t... Is>
    inline Ret call(const std::tuple<Args...>& tuple, std::index_sequence<Is...>) noexcept {
        return handle(std::get<Is>(tuple)...);
    }

public:
    promise_resolver(Ret(*handle)(Args...)): handle{handle} {}

    inline void set_args(Args ... values) {
        arguments = std::tuple<Args...>(values...);
    }

    inline void set_args(std::tuple<Args...> args) {
        arguments = args;
    }

    static void resolve(promise<Ret>* p, promise_resolver<Ret, Args...>& inv) noexcept {
        p->promise_value = inv.call(inv.arguments, std::index_sequence_for<Args...>{});
        p->is_resolved = true;
    }


};


#endif