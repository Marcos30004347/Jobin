#ifndef JOBIN_THREAD_H
#define JOBIN_THREAD_H

#include "config.hpp"

#ifdef THREAD_PTHREAD_BACKEND
#include "thread_pthread_imp.hpp"
#elif THREAD_WINDOWS_BACKEND
#include "thread_windows_imp.hpp"
#endif

#endif