/*
 * Some various functions for getting timestamps.
 *
 * Many thanks to online sources, like:
 *   http://stackoverflow.com/questions/5167269/clock-gettime-alternative-in-mac-os-x
 *   https://janus.conf.meetecho.com/docs/mach__gettime_8h_source.html
 *   http://nadeausoftware.com/articles/2012/04/c_c_tip_how_measure_elapsed_real_time_benchmarking
 *   http://stackoverflow.com/questions/5404277/porting-clock-gettime-to-windows
 *   http://stackoverflow.com/questions/5801813/c-usleep-is-obsolete-workarounds-for-windows-mingw
 */
#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <time.h>

/* begin platform-specific headers and definitions */
#if defined(__MACH__)

#include <mach/clock.h>
#include <mach/mach.h>

#elif defined(_WIN32)

#include <Windows.h>

#else

static const clockid_t PTIME_CLOCKID_T_MONOTONIC =
#if defined(CLOCK_MONOTONIC_PRECISE)
  // BSD
  CLOCK_MONOTONIC_PRECISE
#elif defined(CLOCK_MONOTONIC_RAW)
  // Linux
  CLOCK_MONOTONIC_RAW
#elif defined(CLOCK_HIGHRES)
  // Solaris
  CLOCK_HIGHRES;
#elif defined(CLOCK_MONOTONIC)
  // AIX, BSD, Linux, POSIX, Solaris
  CLOCK_MONOTONIC
#else
  // AIX, BSD, HP-UX, Linux, POSIX
  CLOCK_REALTIME
#endif
;

#endif
/* end platform-specific headers and definitions */

#define ONE_THOUSAND 1000
#define ONE_MILLION  1000000
#define ONE_BILLION  1000000000

#if defined(__MACH__)
static int clock_gettime_mach(clock_id_t clk_id, struct timespec* ts) {
  // OS X does not have clock_gettime, use clock_get_time
  clock_serv_t cclock;
  mach_timespec_t mts;
  int ret;
  host_get_clock_service(mach_host_self(), clk_id, &cclock);
  ret = clock_get_time(cclock, &mts);
  mach_port_deallocate(mach_task_self(), cclock);
  if (!ret) {
    ts->tv_sec = mts.tv_sec;
    ts->tv_nsec = mts.tv_nsec;
  }
  return ret;
}
#endif // __MACH__


#if defined(_WIN32)
static int clock_gettime_win32(int dummy, struct timespec* ts) {
  (void) dummy;
  static LONG g_first_time = 1;
  static LARGE_INTEGER g_counts_per_sec;
  LARGE_INTEGER count;
  // thread-safe initializer
  if (InterlockedExchange(&g_first_time, 0)) {
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

static int ptime_sleep_us_win32(__int64 usec) {
  HANDLE timer;
  LARGE_INTEGER ft;
  // Convert to 100 nanosecond interval, negative value indicates relative time
  ft.QuadPart = -(10 * usec);
  if ((timer = CreateWaitableTimer(NULL, TRUE, NULL)) == NULL) {
    return -1;
  }
  if (!SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0)) {
    CloseHandle(timer);
    return -1;
  }
  WaitForSingleObject(timer, INFINITE);
  CloseHandle(timer);
  return 0;
}
#endif // _WIN32

int ptime_clock_gettime(struct timespec* ts) {
#if defined(__MACH__)
  return clock_gettime_mach(CALENDAR_CLOCK, ts);
#elif defined(_WIN32)
  return clock_gettime_win32(0, ts);
#else
  return clock_gettime(CLOCK_REALTIME, ts);
#endif
}

int ptime_clock_gettime_monotonic(struct timespec* ts) {
#if defined(__MACH__)
  return clock_gettime_mach(SYSTEM_CLOCK, ts);
#elif defined(_WIN32)
  return clock_gettime_win32(0, ts);
#else
  return clock_gettime(PTIME_CLOCKID_T_MONOTONIC, ts);
#endif
}

uint64_t ptime_to_ns(struct timespec* ts) {
  // must cast as a simple literal can overflow!
  return ts->tv_sec * (uint64_t) ONE_BILLION + ts->tv_nsec;
}

uint64_t ptime_gettime_ns(void) {
  struct timespec ts;
  if (ptime_clock_gettime(&ts)) {
    return 0;
  }
  return ptime_to_ns(&ts);
}

int ptime_clock_nanosleep(struct timespec* ts) {
  // TODO: Sleep for any remaining time if interrupted
#if defined(__MACH__)
  return nanosleep(ts, NULL);
#elif defined(_WIN32)
  // TODO: find a more precise approach
  return ptime_sleep_us_win32(ts->tv_sec * 1e6 + ts->tv_nsec / 1e3);
#else
  return clock_nanosleep(CLOCK_REALTIME, 0, ts, NULL);
#endif
}

int ptime_sleep_us(uint64_t us) {
  struct timespec ts;
  if (us == 0) {
    return 0;
  }
  ts.tv_sec = us / (time_t) ONE_MILLION;
  ts.tv_nsec = (us % (long) ONE_MILLION) * (long) ONE_THOUSAND;
  return ptime_clock_nanosleep(&ts);
}

int ptime_sleep_us_monotonic(uint64_t us) {
#if defined(__MACH__) || defined(_WIN32)
  // TODO: monotonic sleeping
  return ptime_sleep_us(us);
#else
  int ret;
  struct timespec ts;
  ts.tv_sec = us % (time_t) ONE_THOUSAND;
  if (us == 0) {
    return 0;
  }
  if (ptime_clock_gettime_monotonic(&ts)) {
    return -1;
  }
  ts.tv_sec += us / (time_t) ONE_MILLION;
  ts.tv_nsec += (us % (long) ONE_MILLION) * (long) ONE_THOUSAND;
  if (ts.tv_nsec >= (long) ONE_BILLION) {
    ts.tv_nsec -= (long) ONE_BILLION;
    ts.tv_sec++;
  }
  // if interrupted, continue sleeping until the time in the future
  while ((ret = clock_nanosleep(PTIME_CLOCKID_T_MONOTONIC, TIMER_ABSTIME, &ts, NULL)) == EINTR);
  return ret;
#endif
}
