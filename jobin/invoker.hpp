#ifndef JOBIN_INVOKER_H
#define JOBIN_INVOKER_H

#include<tuple>

/*
template params:
    Ret: return type
    Args: list of function argument types
*/
template<typename Ret,typename... Args>
class invoker {
private:
    std::tuple<Args...> arguments;
    Ret(*handle)(Args...);

    template<std::size_t... Is>
    inline Ret call(const std::tuple<Args...>& tuple, std::index_sequence<Is...>) noexcept {
        return handle(std::get<Is>(tuple)...);
    }

public:
    invoker(Ret(*handle)(Args...)): handle{handle} {}

    inline void set_args(Args ... values) {
        arguments = std::tuple<Args...>(values...);
    }
    
    static Ret invoke(invoker<Ret, Args...>& inv) noexcept {
        return inv.call(inv.arguments, std::index_sequence_for<Args...>{});
    }


};

template< typename Fn, typename ... Args >
void invoke( Fn && fn, Args && ... args) {
    std::forward< Fn >( fn)( std::forward< Args >( args) ... );
}


#endif
