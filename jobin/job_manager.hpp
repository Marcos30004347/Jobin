#ifndef JOBIN_MANAGER_H
#define JOBIN_MANAGER_H

#include <tuple>
#include "job_priority_queue.hpp"
#include "enums.hpp"
#include "spin_lock.hpp"
#include "job.hpp"

class waiting_job {
public:
    job* ref;
    atomic<int> waiting_for;

    waiting_job* next = nullptr;
    waiting_job* parent = nullptr;

    waiting_job(job* job, int waiting_for): waiting_for{waiting_for}, ref{job} {}
    ~waiting_job() {}
};

#include<iostream>
class job_queues {
private:
    job_priority_queue high_priority_queue;
    job_priority_queue medium_priority_queue;
    job_priority_queue low_priority_queue;

public:
    job_queues() {}

    ~job_queues() {
        job* j;
        // while(high_priority_queue.dequeue(j)) { j->~job(); }
        // while(medium_priority_queue.dequeue(j)) { j->~job(); }
        // while(low_priority_queue.dequeue(j)) { j->~job(); }
    }

    bool enqueue(job* j) {
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

    job_queues jobs;

private:
    waiting_job* put_to_wait(job* w, int waiting_for) {
        waiting_job* waiting = new waiting_job(w, waiting_for);
        
        return waiting; 
    }

    void notify(waiting_job* ref) {
        int waiting;
        do {
            waiting = ref->waiting_for.load();
        } while(!ref->waiting_for.compare_exchange_weak(waiting, waiting - 1));

        if(waiting <= 1) {
            jobs.enqueue(ref->ref);        
            delete ref;
        }
    }


public:
    spin_lock system_lock;

    static job_manager* singleton_ptr;

    job_manager() {}
    ~job_manager() {}

    inline static void init() noexcept { singleton_ptr = new job_manager(); }
    inline static void shut_down() noexcept { delete singleton_ptr; }
    inline static job_manager* get_ptr() noexcept { return singleton_ptr; }

    template<typename Ret, typename ...Args>
    promise<Ret>* enqueue_job(Ret(*handle)(Args...), Args... args) {
        promise<Ret>* prm = new promise<Ret>;
        job* _j = new job(prm, handle, args...);
        jobs.enqueue(_j);
        return prm;
    }

    template<typename Ret, typename ...Args>
    promise<Ret>* enqueue_jobs(Ret(*handle)(Args...), std::tuple<Args...> args[], unsigned int count) {
        promise<Ret>* promises = new promise<Ret>[count];
        job* _j;

        for(int i=0; i<count; i++) {
            _j = new job(&promises[i], handle, args[i]);
            jobs.enqueue(_j);
        }

        return promises;
    }

    template<typename Ret, typename ...Args>
    promise<Ret>* enqueue_jobs_and_wait(Ret(*handle)(Args...), std::tuple<Args...> args[], unsigned int count) {
        waiting_job* ref = put_to_wait(current_job, count);
        promise<Ret>* promises = new promise<Ret>[count];

        job* _j;
    
        for(int i=0; i<count; i++) {
            _j = new job(&promises[i], handle, args[i]);
            _j->waiting_for_me = ref;
            jobs.enqueue(_j);
        }

        return_to_worker();

        return promises;
    }


    template<typename Ret, typename ...Args>
    promise<Ret>* enqueue_job_and_wait(Ret(*handle)(Args...), Args... args) {
        waiting_job* ref = put_to_wait(current_job, 1);
        promise<Ret>* p = new promise<Ret>();

        job* _j;
    
        _j = new job(p, handle, args...);
        _j->waiting_for_me = ref;

        jobs.enqueue(_j);
        return_to_worker();

        return p;
    }
};

job_manager* job_manager::singleton_ptr = nullptr;

#endif