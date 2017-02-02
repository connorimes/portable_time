#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <time.h>
#include "ptime.h"

#define SLEEP_TIME_US 1000000

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

  /* Test timing functions */

  if (ptime_clock_gettime(&ts)) {
    perror("ptime_clock_gettime");
    ret = 1;
  }
  printf("ptime_clock_gettime: %ld sec, %ld ns\n", ts.tv_sec, ts.tv_nsec);

  if (ptime_clock_gettime_monotonic(&ts)) {
    perror("ptime_clock_gettime_monotonic");
    ret = 1;
  }
  printf("ptime_clock_gettime_monotonic: %ld sec, %ld ns\n", ts.tv_sec, ts.tv_nsec);

  ns1 = ptime_gettime_ns();
  if (ns1 == 0) {
    fprintf(stderr, "ptime_gettime_ns returned 0\n");
    if (errno) {
      perror("ptime_gettime_ns");
    }
    ret = 1;
  } else {
    printf("ptime_gettime_ns: %"PRIu64" ns\n", ns1);
  }

  // verify conversion
  ns1 = ptime_to_ns(&ts);
  printf("ptime_to_ns: %"PRIu64" ns\n", ns1);

  /* Test sleeping functions */

  ns1 = ptime_gettime_ns();
  ts.tv_sec = SLEEP_TIME_US / 1000000;
  ts.tv_nsec = (SLEEP_TIME_US % 1000000) * 1000;
  printf("Trying ptime_clock_nanosleep...\n");
  if (ptime_clock_nanosleep(&ts)) {
    perror("ptime_clock_nanosleep");
    ret = 1;
  }
  ns2 = ptime_gettime_ns();
  verify_sleep(SLEEP_TIME_US, (ns2 - ns1) / 1000, &ret);

  ns1 = ptime_gettime_ns();
  printf("Trying ptime_sleep_us...\n");
  if (ptime_sleep_us(SLEEP_TIME_US)) {
    perror("ptime_sleep_us");
    ret = 1;
  }
  ns2 = ptime_gettime_ns();
  verify_sleep(SLEEP_TIME_US, (ns2 - ns1) / 1000, &ret);

  return ret;
}
