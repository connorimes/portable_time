#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <time.h>
#include "ptime.h"

#define SLEEP_TIME_US 1000000

#define ONE_BILLION 1000000000
#define ONE_MILLION 1000000
#define ONE_THOUSAND 1000

static const uint64_t ONE_BILLION_U64 = (uint64_t) ONE_BILLION;
static const uint64_t ONE_MILLION_U64 = (uint64_t) ONE_MILLION;
static const uint64_t ONE_THOUSAND_U64 = (uint64_t) ONE_THOUSAND;

#define SEC 3
#define NSEC 987654321
#define USEC_NSEC NSEC / ONE_THOUSAND * ONE_THOUSAND

static const time_t SEC_U64 = (uint64_t) SEC;
static const uint64_t NSEC_U64 = (uint64_t) NSEC;
static const uint64_t TOTAL_NSEC_U64 = (uint64_t) SEC * ONE_BILLION + NSEC;
static const uint64_t TOTAL_USEC_U64 = (uint64_t) SEC * ONE_MILLION + NSEC / ONE_THOUSAND;

static void verify_sleep(uint64_t expected_us, uint64_t actual_us, int* ret) {
  if (actual_us < expected_us) {
    // sleep failed
    fprintf(stderr, "Only slept for %"PRIu64" us\n", actual_us);
    *ret = 1;
  } else if (actual_us > (expected_us * 10)) {
    // we slept for way too long - something might be wrong
    fprintf(stderr, "Slept for an unexpectedly high %"PRIu64" us\n", actual_us);
    *ret = 1;
  }
}

int main(void) {
  struct timespec ts;
  uint64_t ns1, ns2;
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

  if (ptime_clock_gettime(PTIME_MONOTONIC, &ts)) {
    perror("ERROR: ptime_clock_gettime:PTIME_MONOTONIC");
    ret = 1;
  }
  printf("ptime_clock_gettime:PTIME_MONOTONIC: %ld sec, %ld ns\n", ts.tv_sec, ts.tv_nsec);

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

  /* Test sleeping functions */

  ptime_us_to_timespec(SLEEP_TIME_US, &ts);

  ns1 = ptime_gettime_ns(PTIME_MONOTONIC);
  printf("Trying ptime_clock_nanosleep...\n");
  if (ptime_clock_nanosleep(&ts, NULL)) {
    perror("ptime_clock_nanosleep");
    ret = 1;
  }
  ns2 = ptime_gettime_ns(PTIME_MONOTONIC);
  verify_sleep(SLEEP_TIME_US, (ns2 - ns1) / ONE_THOUSAND_U64, &ret);

  ns1 = ptime_gettime_ns(PTIME_MONOTONIC);
  printf("Trying ptime_sleep_us...\n");
  if (ptime_sleep_us(SLEEP_TIME_US)) {
    perror("ptime_sleep_us");
    ret = 1;
  }
  ns2 = ptime_gettime_ns(PTIME_MONOTONIC);
  verify_sleep(SLEEP_TIME_US, (ns2 - ns1) / ONE_THOUSAND_U64, &ret);

  ns1 = ptime_gettime_ns(PTIME_MONOTONIC);
  printf("Trying ptime_sleep_us_no_interrupt...\n");
  if (ptime_sleep_us_no_interrupt(SLEEP_TIME_US)) {
    perror("ptime_sleep_us_no_interrupt");
    ret = 1;
  }
  ns2 = ptime_gettime_ns(PTIME_MONOTONIC);
  verify_sleep(SLEEP_TIME_US, (ns2 - ns1) / ONE_THOUSAND_U64, &ret);

  return ret;
}
