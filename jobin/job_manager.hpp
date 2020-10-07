#ifndef JOBIN_MANAGER_H
#define JOBIN_MANAGER_H

#include <tuple>
#include "job.hpp"
#include "enums.hpp"
#include "job_priority_queue.hpp"

class waiting_job {
public:
    job* ref;
    atomic<int> waiting_for;

    waiting_job* next = nullptr;
    waiting_job* parent = nullptr;

    waiting_job(job* j, int waiting_for): waiting_for{waiting_for}, ref{j} {}
    ~waiting_job() {}
};

class job_queues {
private:
    job_priority_queue high_priority_queue;
    job_priority_queue medium_priority_queue;
    job_priority_queue low_priority_queue;

public:
    job_queues() {}

    ~job_queues() {}

    bool enqueue(job* const &j) {
        if(j->priority == job_prioriry::HIGH) return high_priority_queue.enqueue(j);
        else if(j->priority == job_prioriry::MEDIUM) return medium_priority_queue.enqueue(j);
        else return low_priority_queue.enqueue(j);
    }

    bool dequeue(job* &j) {
        if(high_priority_queue.dequeue(j)) return true;
        else if(medium_priority_queue.dequeue(j)) return true;
        else return low_priority_queue.dequeue(j);
    }
};


class job_manager {
    friend class worker;
    friend void current_job_yield();
    friend void notify_caller(job* job);

    static job_queues queues;

    // static job_manager* singleton_ptr;

private:
    static waiting_job* put_to_wait(job* w, int waiting_for) {
        waiting_job* waiting = new waiting_job(w, waiting_for);
        
        return waiting; 
    }

    static void notify(waiting_job* ref) {
        int waiting;
        do {
            waiting = ref->waiting_for.load();
        } while(!ref->waiting_for.compare_exchange_weak(waiting, waiting - 1));

        if(waiting <= 1) {
            queues.enqueue(ref->ref);        
            delete ref;
        }
    }


public:
    job_manager() = default;
    ~job_manager() = default;

    // static void init() { if(!singleton_ptr) singleton_ptr = new job_manager(); }
    // static void shut_down() { if(singleton_ptr) delete singleton_ptr; }

    // static job_manager* get_singleton_ptr() {
    //     printf("ta tirano\n");
    //     return singleton_ptr;
    // }

    template<typename Ret, typename ...Args>
    static void enqueue_job(promise<Ret>* p, Ret(*handle)(Args...), Args... args) {
        job* _j = new job(p, handle, args...);
        queues.enqueue(_j);
    }

    template<typename Ret, typename ...Args>
    static void enqueue_jobs(promise<Ret>* promises, Ret(*handle)(Args...), std::tuple<Args...> args[], unsigned int count) {
        job* _j;

        for(int i=0; i<count; i++) {
            _j = new job(&promises[i], handle, args[i]);
            queues.enqueue(_j);
        }
    }

    template<typename Ret, typename ...Args>
    static void enqueue_jobs_and_wait(promise<Ret>* promises, Ret(*handle)(Args...), std::tuple<Args...> args[], unsigned int count) {
        waiting_job* ref = put_to_wait(get_current_job(), count);
        job* _j;
    
        for(int i=0; i<count; i++) {
            _j = new job(&promises[i], handle, args[i]);
            _j->waiting_for_me = ref;
            queues.enqueue(_j);
        }

        return_to_worker();
    }


    template<typename Ret, typename ...Args>
    static void enqueue_job_and_wait(promise<Ret>* p, Ret(*handle)(Args...), Args... args) {
        p->is_resolved = false;
        waiting_job* ref = put_to_wait(get_current_job(), 1);

        job* J;
    
        J = new job(p, handle, args...);
        J->waiting_for_me = ref;

        queues.enqueue(J);

        return_to_worker();
    }
};

#endif