#ifndef _PTIME_H_
#define _PTIME_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <time.h>

#if defined(_WIN32)
struct timespec {
  long int tv_sec;
  long int tv_nsec;
};
#endif

int ptime_clock_gettime(struct timespec* ts);

uint64_t ptime_to_ns(struct timespec* ts);

uint64_t ptime_gettime_ns(void);

#ifdef __cplusplus
}
#endif

#endif
