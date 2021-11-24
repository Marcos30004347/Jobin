#ifndef JOBIN_MANAGER_H
#define JOBIN_MANAGER_H

#include <tuple>

#include "job.hpp"
#include "job_queues.hpp"
#include "waiting_job.hpp"

class job_manager {
  friend class worker;
  friend void current_job_yield();
  friend void notify_caller(job *job);

  static job_queues queues;

private:
  static waiting_job *put_to_wait(job *w, int waiting_for) {
    waiting_job *waiting = new waiting_job(w, waiting_for);

    return waiting;
  }

  static void notify(waiting_job *ref) {
    int waiting;
    do {
      waiting = ref->waiting_for.load();
    } while (!ref->waiting_for.compare_exchange_weak(waiting, waiting - 1));

    if (waiting <= 1) {
      queues.enqueue(ref->ref);
      delete ref;
    }
  }

public:
  job_manager() = default;
  ~job_manager() = default;

  template <typename Ret, typename... Args>
  static void enqueue_job(promise<Ret> *p, Ret (*handle)(Args...),
                          Args &&... args) {
    p->is_resolved = false;
    job *_j = new job(p, handle, std::forward<Args>(args)...);
    queues.enqueue(_j);
  }

  // template<typename Ret, typename ...Args>
  // static void enqueue_jobs(promise<Ret>* promises, Ret(*handle)(Args...),
  // std::tuple<Args...> args[], unsigned int count) {
  //     job* _j;

  //     for(int i=0; i<count; i++) {
  //         promises[i].is_resolved = false;
  //         _j = new job(&promises[i], handle, args[i]);
  //         queues.enqueue(_j);
  //     }
  // }

  // template<typename Ret, typename ...Args>
  // static void enqueue_jobs_and_wait(promise<Ret>* promises,
  // Ret(*handle)(Args...), std::tuple<Args...> args[], unsigned int count) {
  //     waiting_job* ref = put_to_wait(get_current_job(), count);
  //     job* _j;

  //     for(int i=0; i<count; i++) {
  //         promises[i].is_resolved = false;
  //         _j = new job(&promises[i], handle, args[i]);
  //         _j->waiting_for_me = ref;
  //         queues.enqueue(_j);
  //     }

  //     return_to_worker();
  // }

  template <typename Ret, typename... Args>
  static void enqueue_job_and_wait(promise<Ret> *p, Ret (*handle)(Args...),
                                   Args &&... args) {
    p->is_resolved = false;
    waiting_job *ref = put_to_wait(get_current_job(), 1);

    job *J;

    J = new job(p, handle, std::forward<Args>(args)...);
    J->waiting_for_me = ref;

    queues.enqueue(J);

    return_to_worker();
  }
};

#endif
