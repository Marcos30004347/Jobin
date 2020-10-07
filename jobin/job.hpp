#ifndef JOBIN_WORK_H
#define JOBIN_WORK_H

#include <string.h>

#include "fiber.hpp"
#include "fiber_pool_fixed.hpp"
#include "enums.hpp"
#include "spin_lock.hpp"
#include "promise_resolver.hpp"

class job;
class waiting_job;

thread_local static job* current_job = nullptr;

void return_to_worker();
void notify_caller(job* job);
void current_job_yield();

using job_invoke_method = unsigned int (*)(void*, void*);

class job {
    friend class worker;
    friend class job_queues;
    friend class job_manager;
    friend class job_waiting_list;
    friend class job_avaliable_queue;

    friend void notify_caller(job* job);

    waiting_job* waiting_for_me = nullptr;
    fiber* context = nullptr;
    fiber_pool_fixed<256>* pool = nullptr;

    job_status status = UNINITIATED;
    job_prioriry priority = HIGH;

    job_invoke_method handler; 
 
    spin_lock lock;

    void* resolver_ptr = nullptr;
    void* promise_ptr = nullptr;

    public:


    /**
     * job contructor.
     * @param promise: promise to assign to this job.
     * @param handle: job handle.
     * @param args: job arguments.
     */
    template<typename Ret, typename ...Args>
    job(promise<Ret>* promise, Ret(*handle)(Args...), Args... args): status{UNINITIATED}, priority{HIGH} {
        promise_resolver<Ret, Args...>* resolver = (promise_resolver<Ret, Args...>*)malloc(sizeof(promise_resolver<Ret, Args...>));
        resolver->handle = handle;
        resolver->set_args(std::forward<Args>(args)...);
        resolver_ptr = resolver;
        promise_ptr = promise;
        handler = reinterpret_cast<job_invoke_method>(resolver->resolve_with_resolver);
    }

    /**
     * job contructor.
     * @param promise: promise to assign to this job.
     * @param handle: job handle.
     * @param args: job arguments.
     */
    template<typename Ret, typename ...Args>
    job(promise<Ret>* promise, Ret(*handle)(Args...), std::tuple<Args...> args): status{UNINITIATED}, priority{HIGH} {
        promise_resolver<Ret, Args...>* resolver = (promise_resolver<Ret, Args...>*)malloc(sizeof(promise_resolver<Ret, Args...>));
        resolver->handle = handle;
        resolver->set_args(std::forward<std::tuple<Args...>>(args));
        resolver_ptr = resolver;
        promise_ptr = promise;
        handler = reinterpret_cast<job_invoke_method>(resolver->resolve_with_resolver);
    }

    /**
     * job destructor.
     */
    ~job() {
        free(resolver_ptr);
    }

    /**
     * Execute @j job.
     * @param j: job to execute.
     */
    static void execute(void* j) {
        job* w = reinterpret_cast<job*>(j);
        job_invoke_method call = (job_invoke_method)w->handler;
        w->status = job_status::RUNNING;
        call(w->promise_ptr, w->resolver_ptr);

        w->status = job_status::FINISHING;
        notify_caller(w);
        return_to_worker();
    }
};



#endif
