#ifndef JOBIN_JOB_H
#define JOBIN_JOB_H


template<typename Ret,typename... Args>
class job_handle {
    friend class job;
    Ret(*handle)(Args...);
public:
    job_handle(Ret(*handle)(Args...)):handle{handle} {}
};

#endif
