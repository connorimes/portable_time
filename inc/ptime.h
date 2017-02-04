#ifndef _PTIME_H_
#define _PTIME_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <time.h>

typedef enum ptime_clock_id {
  PTIME_REALTIME = 0,
  PTIME_MONOTONIC
} ptime_clock_id;

/*
 * Conversions
 */

uint64_t ptime_timespec_to_ns(struct timespec* ts);
uint64_t ptime_timespec_to_us(struct timespec* ts);
void ptime_ns_to_timespec(uint64_t ns, struct timespec* ts);
void ptime_us_to_timespec(uint64_t us, struct timespec* ts);

/*
 * Get time
 */

int ptime_clock_gettime(ptime_clock_id clk_id, struct timespec* ts);
uint64_t ptime_gettime_ns(ptime_clock_id clk_id);
uint64_t ptime_gettime_us(ptime_clock_id clk_id);
int64_t ptime_gettime_elapsed_ns(ptime_clock_id clk_id, struct timespec* ts);
int64_t ptime_gettime_elapsed_us(ptime_clock_id clk_id, struct timespec* ts);

/*
 * Sleeping
 */

int ptime_nanosleep(struct timespec* ts, struct timespec* rem);
int ptime_sleep_us(uint64_t us);
int ptime_sleep_us_no_interrupt(uint64_t us);

#ifdef __cplusplus
}
#endif

#endif
