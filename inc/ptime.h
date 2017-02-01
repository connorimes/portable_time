#ifndef _PTIME_H_
#define _PTIME_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <time.h>

int ptime_clock_gettime(struct timespec* ts);

uint64_t ptime_to_ns(struct timespec* ts);

uint64_t ptime_gettime_ns(void);

#ifdef __cplusplus
}
#endif

#endif
