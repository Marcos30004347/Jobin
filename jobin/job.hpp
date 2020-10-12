#ifndef JOBIN_WORK_H
#define JOBIN_WORK_H

#include <string.h>

#include "fiber.hpp"
#include "fiber_pool_fixed.hpp"
#include "spin_lock.hpp"
#include "promise_resolver.hpp"
#include "waiting_job.hpp"

enum job_prioriry {
    HIGH,
    MEDIUM,
    LOW
};

enum job_status {
    UNINITIATED = 0,
    WAITING,
    RUNNING,
    FINISHING,
};


class job;

void return_to_worker();
void notify_caller(job* job);
void current_job_yield();
job* get_current_job() noexcept;
void set_current_job(job* j) noexcept;

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

    /**
     * Execute @j job.
     * @param j: job to execute.
     */
    static void execute(void* j);
    public:


    /**
     * job contructor.
     * @param promise: promise to assign to this job.
     * @param handle: job handle.
     * @param args: job arguments.
     */
    template<typename Ret, typename ...Args>
    job(promise<Ret>* promise, Ret(*handle)(Args...), Args&&... args): status{UNINITIATED}, priority{HIGH} {
        promise_resolver<Ret, Args...>* resolver = new promise_resolver<Ret, Args...>(handle, std::forward<Args>(args)...);
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
        promise_resolver<Ret, Args...>* resolver = new promise_resolver<Ret, Args...>(handle, std::forward<std::tuple<Args...>>(args));
        resolver_ptr = resolver;
        promise_ptr = promise;
        handler = reinterpret_cast<job_invoke_method>(resolver->resolve_with_resolver);
    }

    job() {}

    /**
     * job destructor.
     */
    ~job() {
        delete (unsigned int*)resolver_ptr;
    }

};


#endif
