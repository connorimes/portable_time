#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <time.h>
#include "ptime.h"

#define SLEEP_TIME_US 1000000

#define ONE_BILLION 1000000000
#define ONE_MILLION 1000000
#define ONE_THOUSAND 1000

#define SEC 3
#define NSEC 987654321
#define USEC_NSEC NSEC / ONE_THOUSAND * ONE_THOUSAND

static const time_t SEC_U64 = (uint64_t) SEC;
static const uint64_t NSEC_U64 = (uint64_t) NSEC;
static const uint64_t TOTAL_NSEC_U64 = (uint64_t) SEC * ONE_BILLION + NSEC;
static const uint64_t TOTAL_USEC_U64 = (uint64_t) SEC * ONE_MILLION + NSEC / ONE_THOUSAND;

static const uint64_t MAX_DUMMY_ITERS = (uint64_t) ONE_MILLION;

static void dummy_work() {
  uint64_t i;
  for (i = 0; i < MAX_DUMMY_ITERS; i++);
}

/**
 * Not all systems are very precise.
 * Enforce that we slept at least 95% of, and not more than 5x, the requested time.
 * Sleep functions usually guarantee a min sleep time (excluding interrupts), so 105% of the requested time may not be
 * sufficiently tight.
 * This at least keeps us within an order of magnitude, which are likely coding errors to make.
 */
static void verify_sleep(uint64_t expected_us, uint64_t actual_us, int* ret) {
  if ((double) actual_us + (expected_us / 20.0) < (double) expected_us) {
    // sleep failed - more than 5% of time was remaining
    fprintf(stderr, "Only slept for %"PRIu64" us\n", actual_us);
    *ret = 1;
  } else if (actual_us > (expected_us * 5)) {
    // we slept for way too long - something might be wrong
    fprintf(stderr, "Slept for an unexpectedly high %"PRIu64" us\n", actual_us);
    *ret = 1;
  }
}

int main(void) {
  struct timespec ts;
  uint64_t ns1, ns2;
  int64_t elapsed;
  int ret = 0;

  /* Test conversions */

  ts.tv_sec = SEC_U64;
  ts.tv_nsec = NSEC_U64;
  ns1 = ptime_timespec_to_ns(&ts);
  printf("ptime_timespec_to_ns: %"PRIu64" ns\n", ns1);
  if (ns1 != TOTAL_NSEC_U64) {
    fprintf(stderr, "ERROR: ptime_timespec_to_ns\n");
    ret = 1;
  }
  ns1 = ptime_timespec_to_us(&ts);
  printf("ptime_timespec_to_us: %"PRIu64" us\n", ns1);
  if (ns1 != TOTAL_USEC_U64) {
    fprintf(stderr, "ERROR: ptime_timespec_to_us\n");
    ret = 1;
  }
  ptime_ns_to_timespec(TOTAL_NSEC_U64, &ts);
  if (ts.tv_sec != SEC || ts.tv_nsec != NSEC) {
    fprintf(stderr, "ERROR: ptime_ns_to_timespec\n");
    ret = 1;
  }
  ptime_us_to_timespec(TOTAL_USEC_U64, &ts);
  if (ts.tv_sec != SEC || ts.tv_nsec != USEC_NSEC) {
    fprintf(stderr, "ERROR: ptime_us_to_timespec\n");
    ret = 1;
  }

  /* Test timing functions */

  if (ptime_clock_gettime(PTIME_REALTIME, &ts)) {
    perror("ERROR: ptime_clock_gettime:PTIME_REALTIME");
    ret = 1;
  }
  printf("ptime_clock_gettime:PTIME_REALTIME: %ld sec, %ld ns\n", ts.tv_sec, ts.tv_nsec);

  dummy_work();

  elapsed = ptime_gettime_elapsed_ns(PTIME_REALTIME, &ts);
  printf("ptime_gettime_elapsed_ns:PTIME_REALTIME: %"PRIi64" ns\n", elapsed);
  if (elapsed == 0 && errno) {
  	perror("ptime_gettime_elapsed_ns:PTIME_REALTIME");
  }

  if (ptime_clock_gettime(PTIME_MONOTONIC, &ts)) {
    perror("ERROR: ptime_clock_gettime:PTIME_MONOTONIC");
    ret = 1;
  }
  printf("ptime_clock_gettime:PTIME_MONOTONIC: %ld sec, %ld ns\n", ts.tv_sec, ts.tv_nsec);

  dummy_work();

  elapsed = ptime_gettime_elapsed_us(PTIME_MONOTONIC, &ts);
  printf("ptime_gettime_elapsed_us:PTIME_MONOTONIC: %"PRIi64" us\n", elapsed);
  if (elapsed == 0 && errno) {
  	perror("ptime_gettime_elapsed_us:PTIME_MONOTONIC");
  }

  ns1 = ptime_gettime_ns(PTIME_REALTIME);
  if (ns1 == 0) {
    perror("ERROR: ptime_gettime_ns:PTIME_REALTIME returned 0");
    ret = 1;
  }
  printf("ptime_gettime_ns:PTIME_REALTIME: %"PRIu64" ns\n", ns1);

  ns1 = ptime_gettime_ns(PTIME_MONOTONIC);
  if (ns1 == 0) {
    perror("ERROR: ptime_gettime_ns:PTIME_MONOTONIC returned 0");
    ret = 1;
  }
  printf("ptime_gettime_ns:PTIME_MONOTONIC: %"PRIu64" ns\n", ns1);


  ns1 = ptime_gettime_us(PTIME_REALTIME);
  if (ns1 == 0) {
    perror("ERROR: ptime_gettime_us:PTIME_REALTIME returned 0");
    ret = 1;
  }
  printf("ptime_gettime_us:PTIME_REALTIME: %"PRIu64" us\n", ns1);

  ns1 = ptime_gettime_us(PTIME_MONOTONIC);
  if (ns1 == 0) {
    perror("ERROR: ptime_gettime_us:PTIME_MONOTONIC returned 0");
    ret = 1;
  }
  printf("ptime_gettime_us:PTIME_MONOTONIC: %"PRIu64" us\n", ns1);

  /* Test sleeping functions */

  ptime_us_to_timespec(SLEEP_TIME_US, &ts);

  ns1 = ptime_gettime_us(PTIME_MONOTONIC);
  printf("Trying ptime_nanosleep...\n");
  if (ptime_nanosleep(&ts, NULL)) {
    perror("ptime_nanosleep");
    ret = 1;
  }
  ns2 = ptime_gettime_us(PTIME_MONOTONIC);
  verify_sleep(SLEEP_TIME_US, (ns2 - ns1), &ret);

  ns1 = ptime_gettime_us(PTIME_MONOTONIC);
  printf("Trying ptime_sleep_us...\n");
  if (ptime_sleep_us(SLEEP_TIME_US)) {
    perror("ptime_sleep_us");
    ret = 1;
  }
  ns2 = ptime_gettime_us(PTIME_MONOTONIC);
  verify_sleep(SLEEP_TIME_US, (ns2 - ns1), &ret);

  ns1 = ptime_gettime_us(PTIME_MONOTONIC);
  printf("Trying ptime_sleep_us_no_interrupt...\n");
  if (ptime_sleep_us_no_interrupt(SLEEP_TIME_US)) {
    perror("ptime_sleep_us_no_interrupt");
    ret = 1;
  }
  ns2 = ptime_gettime_us(PTIME_MONOTONIC);
  verify_sleep(SLEEP_TIME_US, (ns2 - ns1), &ret);

  return ret;
}
