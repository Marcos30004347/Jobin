#ifndef JOBIN_WORK_QUEUES_LIST
#define JOBIN_WORK_QUEUES_LIST

// #include "job.hpp"
#include "ring_buffer.hpp"

class job;
using job_priority_queue = ring_buffer<job*, 1024>;


#endif