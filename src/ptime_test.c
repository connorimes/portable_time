#include <inttypes.h>
#include <stdio.h>
#include <time.h>
#include "ptime.h"

#define SLEEP_TIME_US 1000000

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

  // verify conversion
  ns1 = ptime_to_ns(&ts);
  printf("ptime_to_ns: %"PRIu64" ns\n", ns1);

  // try a sleep
  // ptime_sleep_us(SLEEP_TIME_US);
  // ts.tv_sec = 1;
  // ts.tv_nsec = 0;
  // if (ptime_clock_nanosleep(&ts)) {
  //   perror("ptime_clock_nanosleep");
  //   ret = 1;
  // }
  if (ptime_sleep_us(SLEEP_TIME_US)) {
    perror("ptime_sleep_us");
    ret = 1;
  }

  // verify gettime_ns while simultaneously showing that we slept correctly
  ns2 = ptime_gettime_ns();
  printf("ptime_gettime_ns: %"PRIu64" ns\n", ns2);

  if ((ns2 - ns1) < SLEEP_TIME_US) {
    // sleep failed
    fprintf(stderr, "Only slept for %"PRIu64" ns\n", ns2 - ns1);
    ret = 1;
  }
  return ret;
}
