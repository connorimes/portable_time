#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <time.h>
#include "ptime.h"

#define SLEEP_TIME_US 1000000

static void verify_sleep(uint64_t expected, uint64_t actual, int* ret) {
  if (actual < expected) {
    // sleep failed
    fprintf(stderr, "Only slept for %"PRIu64" ns\n", actual);
    *ret = 1;
  } else if (actual > expected * 10) {
    // we slept for way too long - something might be wrong
    fprintf(stderr, "Slept for an unexpectedly high %"PRIu64" ns\n", actual);
    *ret = 1;
  }
}

int main(void) {
  struct timespec ts;
  uint64_t ns1, ns2;
  int ret = 0;

  // verify gettime
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

  // verify conversion
  ns1 = ptime_to_ns(&ts);
  printf("ptime_to_ns: %"PRIu64" ns\n", ns1);

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

  // try sleep functions
  ts.tv_sec = SLEEP_TIME_US / 1000000;
  ts.tv_nsec = (SLEEP_TIME_US % 1000000) * 1000;
  printf("Trying ptime_clock_nanosleep...\n");
  if (ptime_clock_nanosleep(&ts)) {
    perror("ptime_clock_nanosleep");
    ret = 1;
  }
  ns2 = ptime_gettime_ns();
  verify_sleep(SLEEP_TIME_US, ns2 - ns1, &ret);

  ns1 = ns2;
  printf("Trying ptime_sleep_us...\n");
  if (ptime_sleep_us(SLEEP_TIME_US)) {
    perror("ptime_sleep_us");
    ret = 1;
  }
  ns2 = ptime_gettime_ns();
  verify_sleep(SLEEP_TIME_US, ns2 - ns1, &ret);

  return ret;
}
