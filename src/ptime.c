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

#if defined(_WIN32)
// See http://stackoverflow.com/questions/5404277/porting-clock-gettime-to-windows

static LARGE_INTEGER getFILETIMEoffset() {
  SYSTEMTIME s;
  FILETIME f;
  LARGE_INTEGER t;

  s.wYear = 1970;
  s.wMonth = 1;
  s.wDay = 1;
  s.wHour = 0;
  s.wMinute = 0;
  s.wSecond = 0;
  s.wMilliseconds = 0;
  SystemTimeToFileTime(&s, &f);
  t.QuadPart = f.dwHighDateTime;
  t.QuadPart <<= 32;
  t.QuadPart |= f.dwLowDateTime;
  return t;
}

static int clock_gettime_win32(int dummy, struct timespec *ts) {
  (void) dummy;
  LARGE_INTEGER           t;
  FILETIME                f;
  double                  microseconds;
  static LARGE_INTEGER    offset;
  static double           frequencyToMicroseconds;
  static int              initialized = 0;
  static BOOL             usePerformanceCounter = 0;

  if (!initialized) {
    LARGE_INTEGER performanceFrequency;
    initialized = 1;
    usePerformanceCounter = QueryPerformanceFrequency(&performanceFrequency);
    if (usePerformanceCounter) {
      QueryPerformanceCounter(&offset);
      frequencyToMicroseconds = (double)performanceFrequency.QuadPart / 1000000.;
    } else {
      offset = getFILETIMEoffset();
      frequencyToMicroseconds = 10.;
    }
  }
  if (usePerformanceCounter) {
    QueryPerformanceCounter(&t);
  } else {
    GetSystemTimeAsFileTime(&f);
    t.QuadPart = f.dwHighDateTime;
    t.QuadPart <<= 32;
    t.QuadPart |= f.dwLowDateTime;
  }

  t.QuadPart -= offset.QuadPart;
  microseconds = (double)t.QuadPart / frequencyToMicroseconds;
  t.QuadPart = microseconds;
  ts->tv_sec = t.QuadPart / 1000000;
  ts->tv_usec = t.QuadPart % 1000000;
  return 0;
}
#endif

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
  return clock_gettime_win32(0, ts);
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
  struct timespec ts;
  clock_gettime_win32(0, &ts);
  return ts.tv_sec * ONE_BILLION_U64 + ts.tv_nsec;
#else
  struct timespec ts;
  // CLOCK_REALTIME is always supported, this should never fail
  clock_gettime(CLOCK_REALTIME, &ts);
  // must use a const or cast a literal - using a simple literal can overflow!
  return ts.tv_sec * ONE_BILLION_U64 + ts.tv_nsec;
#endif
}
