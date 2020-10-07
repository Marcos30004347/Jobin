#include"job_queues.hpp"
#include "job_priority_queue.hpp"

job_priority_queue high_priority_queue;
job_priority_queue medium_priority_queue;
job_priority_queue low_priority_queue;

job_queues::job_queues() {}

job_queues::~job_queues() {}

bool job_queues::enqueue(job* const &j) {
    if(j->priority == job_prioriry::HIGH) return high_priority_queue.enqueue(j);
    else if(j->priority == job_prioriry::MEDIUM) return medium_priority_queue.enqueue(j);
    else return low_priority_queue.enqueue(j);
}

bool job_queues::dequeue(job* &j) {
    if(high_priority_queue.dequeue(j)) return true;
    else if(medium_priority_queue.dequeue(j)) return true;
    else return low_priority_queue.dequeue(j);
}

