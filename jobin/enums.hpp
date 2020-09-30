#ifndef JOBIN_ENUMS_H
#define JOBIN_ENUMS_H

enum job_prioriry {
    HIGH,
    MEDIUM,
    LOW
};

enum job_status {
    UNINITIATED = 0,
    WAITING,
    RUNNING,
    FINISHING,
};


#endif