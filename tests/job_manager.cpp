#include "jobin/job_manager.hpp"

#include <assert.h>
#include <iostream>

#define RECORD_ALLOCATED_MEMORY_COUNT
#include "jobin/memory.hpp"

int handle(int a, int b) {
    return a+b;
}

int main() {
    assert(job_manager::get_ptr() == nullptr);

    job_manager::init();

    assert(job_manager::get_ptr());

    job* _job = nullptr;

    assert(job_manager::get_ptr()->high_priority_queue.dequeue(_job) == false);
  
    assert(_job == nullptr);

    promise<int>* p = job_manager::get_ptr()->enqueue_job(handle, 1, 2);

    assert(job_manager::get_ptr()->high_priority_queue.dequeue(_job) == true);
    
    assert(_job != nullptr);

    delete _job;
    delete p;

    job_manager::shut_down();

    assert(memory::currently_allocated_memory_count() == 0);

    return 0;
}