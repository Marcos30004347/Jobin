#include "job.hpp"

thread_local static job* current_job = nullptr;

job* get_current_job() noexcept {
    return current_job;
}

void set_current_job(job* j) noexcept {
    current_job = j;
}

void job::execute(void* j) {
    job* w = reinterpret_cast<job*>(j);

    w->status = job_status::RUNNING;

    w->handler(w->promise_ptr, w->resolver_ptr);

    w->status = job_status::FINISHING;

    notify_caller(w);
    return_to_worker();
}