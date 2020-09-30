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


template<typename... Args>
class artuments_pack {
    friend class job_manager;
private:
    std::tuple<Args...> arguments;

public:
    artuments_pack(Args ... values) {
        arguments = std::tuple<Args...>(values...);
    }
    artuments_pack() {}
};


class job_manager {
public:
    job_priority_queue high_priority_queue;
    job_priority_queue medium_priority_queue;
    job_priority_queue low_priority_queue;

    // waiting_job* waiting_list; 
    spin_lock system_lock;

    static job_manager* singleton_ptr;

    job_manager() {}

    static void init() {
        singleton_ptr = new job_manager();
    }
    
    static void shut_down() {
        // waiting_job* tmp = singleton_ptr->waiting_list;
        
        // while(singleton_ptr->waiting_list) {
        //     tmp = singleton_ptr->waiting_list->parent;
        //     delete singleton_ptr->waiting_list;
        // }

        delete singleton_ptr;
    }
    

    static job_manager* get_ptr() {
        return singleton_ptr;
    }


    waiting_job* put_to_wait(job* w, int waiting_for) {
        waiting_job* waiting = new waiting_job(w, waiting_for);
        
        // system_lock.lock();

        // if(waiting_list) {
        //     waiting_list->next = waiting;
        //     waiting->parent = waiting_list;
        //     waiting_list = waiting;
        // } else {
        //     waiting_list = waiting;
        // }

        // system_lock.unlock();

        return waiting; 
    }

    void notify(waiting_job* ref) {
        int waiting;
        do {
            waiting = ref->waiting_for.load();
        } while(!ref->waiting_for.compare_exchange_weak(waiting, waiting - 1));

        if(waiting <= 1) {
            put_to_run(ref);
        }
    }

    void put_to_run(waiting_job* node) {
        if(node->ref->priority == job_prioriry::HIGH) high_priority_queue.enqueue(node->ref);
        else if(node->ref->priority == job_prioriry::MEDIUM) medium_priority_queue.enqueue(node->ref);
        else if(node->ref->priority == job_prioriry::LOW) low_priority_queue.enqueue(node->ref);
        
        // system check
        // system_lock.lock();
        // if(node->ref == waiting_list) {
        //     waiting_list = waiting_list->next;
        // }

        // if(node->parent) node->parent->next = node->next;
        // if(node->next) node->next->parent = node->parent;
        // system_lock.unlock();

        delete node;
    }

    template<typename Ret, typename ...Args>
    promise<Ret>* enqueue_job(Ret(*handle)(Args...), Args... args) {
        promise<Ret>* prm = new promise<Ret>;
        job* _j = new job(prm, handle, args...);

        if(_j->priority == job_prioriry::HIGH) high_priority_queue.enqueue(_j);
        else if(_j->priority == job_prioriry::MEDIUM) medium_priority_queue.enqueue(_j);
        else if(_j->priority == job_prioriry::LOW) low_priority_queue.enqueue(_j);

        return prm;
    }

    template<typename Ret, typename ...Args>
    promise<Ret>* enqueue_jobs(Ret(*handle)(Args...), std::tuple<Args...> args[], unsigned int count) {
        promise<Ret>* promises = new promise<Ret>[count];
        job* _j;

        for(int i=0; i<count; i++) {

            _j = new job(&promises[i], handle, args[i]);

            if(_j->priority == job_prioriry::HIGH) high_priority_queue.enqueue(_j);
            else if(_j->priority == job_prioriry::MEDIUM) medium_priority_queue.enqueue(_j);
            else if(_j->priority == job_prioriry::LOW) low_priority_queue.enqueue(_j);
        }

        return promises;
    }


    template<typename Ret, typename ...Args>
    promise<Ret>* enqueue_and_wait_jobs(Ret(*handle)(Args...), std::tuple<Args...> args[], unsigned int count) {
        waiting_job* ref = put_to_wait(current_job, count);
        promise<Ret>* promises = new promise<Ret>[count];

        job* _j;
    
        for(int i=0; i<count; i++) {
            _j = new job(&promises[i], handle, args[i]);
            _j->waiting_for_me = ref;

            if(_j->priority == job_prioriry::HIGH) high_priority_queue.enqueue(_j);
            else if(_j->priority == job_prioriry::MEDIUM) medium_priority_queue.enqueue(_j);
            else if(_j->priority == job_prioriry::LOW) low_priority_queue.enqueue(_j);
        }

        return_to_worker();

        return promises;
    }

    void yield() {
        if(current_job->priority == job_prioriry::HIGH) high_priority_queue.enqueue(current_job);
        else if(current_job->priority == job_prioriry::MEDIUM) medium_priority_queue.enqueue(current_job);
        else if(current_job->priority == job_prioriry::LOW) low_priority_queue.enqueue(current_job);
        return_to_worker();
    }

    template<typename Ret>
    void wait_promise(promise<Ret>* p) {
        while (!p->is_resolved) {
            yield();
        }
    }

};

job_manager* job_manager::singleton_ptr = nullptr;

#endif