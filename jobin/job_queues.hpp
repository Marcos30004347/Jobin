#ifndef JOBIN_JOB_QUEUES_H
#define JOBIN_JOB_QUEUES_H

#include "job.hpp"

class job_queues {

public:
    job_queues();
    ~job_queues();
    bool enqueue(job* const &j);
    bool dequeue(job* &j);
};


#endif