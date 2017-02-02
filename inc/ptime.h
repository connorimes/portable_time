#ifndef _PTIME_H_
#define _PTIME_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <time.h>

int ptime_clock_gettime(struct timespec* ts);

int ptime_clock_gettime_monotonic(struct timespec* ts);

uint64_t ptime_to_ns(struct timespec* ts);

uint64_t ptime_gettime_ns(void);

int ptime_clock_nanosleep(struct timespec* ts);

int ptime_sleep_us(uint64_t us);

int ptime_sleep_us_monotonic(uint64_t us);

#ifdef __cplusplus
}
#endif

#endif
