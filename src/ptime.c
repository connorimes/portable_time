/*
 * Some various functions for getting timestamps.
 *
 * Many thanks to online sources, like:
 *   http://stackoverflow.com/questions/5167269/clock-gettime-alternative-in-mac-os-x
 *   http://stackoverflow.com/questions/5404277/porting-clock-gettime-to-windows
 */
#include <inttypes.h>
#include <stdlib.h>
#include <time.h>
#if defined(__MACH__)
#include <mach/clock.h>
#include <mach/mach.h>
#endif
#if defined(_WIN32)
#include <Windows.h>
#endif

#if defined(__MACH__)
static int clock_gettime_mach(int dummy, struct timespec* ts) {
  (void) dummy;
  // OS X does not have clock_gettime, use clock_get_time
  clock_serv_t cclock;
  mach_timespec_t mts;
  host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
  clock_get_time(cclock, &mts);
  mach_port_deallocate(mach_task_self(), cclock);
  ts->tv_sec = mts.tv_sec;
  ts->tv_nsec = mts.tv_nsec;
  return 0;
}
#endif

#if defined(_WIN32)
int clock_gettime_win32(int dummy, struct timespec* ts) {
  static BOOL g_first_time = 1;
  static LARGE_INTEGER g_counts_per_sec;
  LARGE_INTEGER count;
  if (g_first_time) {
    g_first_time = 0;
    if (QueryPerformanceFrequency(&g_counts_per_sec) == 0) {
      g_counts_per_sec.QuadPart = 0;
    }
  }
  if (g_counts_per_sec.QuadPart <= 0 || QueryPerformanceCounter(&count) == 0) {
    return -1;
  }
  ts->tv_sec = count.QuadPart / g_counts_per_sec.QuadPart;
  ts->tv_nsec = ((count.QuadPart % g_counts_per_sec.QuadPart) * 1E9) / g_counts_per_sec.QuadPart;
  return 0;
}
#endif

int ptime_clock_gettime(struct timespec* ts) {
#if defined(__MACH__)
  return clock_gettime_mach(0, ts);
#elif defined(_WIN32)
  return clock_gettime_win32(0, ts);
#else
  return clock_gettime(CLOCK_REALTIME, ts);
#endif
}

uint64_t ptime_to_ns(struct timespec* ts) {
  // must use a const or cast a literal - using a simple literal can overflow!
  static const uint64_t ONE_BILLION_U64 = 1000000000;
  return ts->tv_sec * ONE_BILLION_U64 + ts->tv_nsec;
}

uint64_t ptime_gettime_ns(void) {
  struct timespec ts;
#if defined(__MACH__)
  clock_gettime_mach(0, &ts);
#elif defined(_WIN32)
  clock_gettime_win32(0, &ts);
#else
  // CLOCK_REALTIME is always supported, this should never fail
  clock_gettime(CLOCK_REALTIME, &ts);
#endif
  return ptime_to_ns(&ts);
}
