#include <inttypes.h>
#include <time.h>
#if defined(__MACH__)
#include <mach/clock.h>
#include <mach/mach.h>
#endif
#if defined(_WIN32)
#include <Windows.h>
#endif

// TODO: CLOCK_MONOTONIC

int ptime_clock_gettime(struct timespec* ts) {
#if defined(__MACH__)
  clock_serv_t cclock;
  mach_timespec_t mts;
  host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);
  clock_get_time(cclock, &mts);
  mach_port_deallocate(mach_task_self(), cclock);
  ts->tv_sec = mts.tv_sec;
  ts->tv_nsec = mts.tv_nsec;
  return 0;
#elif defined(_WIN32)
  // TODO
#else
  return clock_gettime(CLOCK_REALTIME, ts);
#endif
}

uint64_t ptime_to_ns(struct timespec* ts) {
  static const uint64_t ONE_BILLION_U64 = 1000000000;
  return ts->tv_sec * ONE_BILLION_U64 + ts->tv_nsec;
}

uint64_t ptime_gettime_ns(void) {
static const uint64_t ONE_BILLION_U64 = 1000000000;
#if defined(__MACH__)
  // OS X does not have clock_gettime, use clock_get_time
  clock_serv_t cclock;
  mach_timespec_t mts;
  host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
  clock_get_time(cclock, &mts);
  mach_port_deallocate(mach_task_self(), cclock);
  return mts.tv_sec * ONE_BILLION_U64 + mts.tv_nsec;
#elif defined(_WIN32)
  // TODO
#else
  struct timespec ts;
  // CLOCK_REALTIME is always supported, this should never fail
  clock_gettime(CLOCK_REALTIME, &ts);
  // must use a const or cast a literal - using a simple literal can overflow!
  return ts.tv_sec * ONE_BILLION_U64 + ts.tv_nsec;
#endif
}
