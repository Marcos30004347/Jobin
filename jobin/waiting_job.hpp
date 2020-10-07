#ifndef JOBIN_WAITING_JOB
#define JOBIN_WAITING_JOB

#include "atomic.hpp"
class job;

class waiting_job {
public:
    job* ref;
    atomic<int> waiting_for;

    waiting_job* next;
    waiting_job* parent;

    waiting_job(job* j, int waiting_for): waiting_for{waiting_for}, ref{j} {}
    ~waiting_job() {}
};


#endif