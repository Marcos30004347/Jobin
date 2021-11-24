#include "thread.hpp"
#include "config.hpp"

#ifdef THREAD_PTHREAD_BACKEND
#include <pthread.h>
#elif THREAD_WINDOWS_BACKEND
#include <winbase.h>
#endif

#include <assert.h>

#if defined(ANDROID) || defined(__ANDROID__)
#include <sched.h>
#endif

thread::thread(void (*fn)(void *data), void *data) : _thread(fn, data) {}
thread::~thread() { _thread.~thread(); }

void thread::join() noexcept { _thread.join(); }
void thread::close() noexcept { this->~thread(); }
void thread::get_id() noexcept { _thread.get_id(); }
void thread::swap_with(thread &other) noexcept { _thread.swap(other._thread); }

bool thread::this_thread::set_affinity(int core) {
#ifdef __APPLE__
#ifdef TARGET_OS_IPHONE
  // TODO
#elif TARGET_IPHONE_SIMULATOR
  // TODO
#elif TARGET_OS_MAC
  // TODO
#else
  // TODO
#endif
#elif THREAD_PTHREAD_BACKEND
  // pthread specific code
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(core, &cpuset);

  pthread_t current_thread = pthread_self();
  int error =
      pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
  if (error)
    return false;
  return true;

#elif THREAD_WINDOWS_BACKEND
  // WinThread specific code
  auto mask = (static_cast<DWORD_PTR>(1) << core);
  auto ret = SetThreadAffinityMask(_thread.native_handle(), mask);
  if (!ret)
    return false;
  return true;

#elif defined(ANDROID) || defined(__ANDROID__)
  cpu_set_t my_set;
  CPU_ZERO(&my_set);
  CPU_SET(7, &my_set);
  sched_setaffinity(0, sizeof(cpu_set_t), &my_set);
#endif
	return false;
}

void thread::this_thread::yield() { std::this_thread::yield(); }
