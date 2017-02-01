#include <inttypes.h>
#include <stdio.h>
#include <time.h>
#include "ptime.h"

int main(void) {
  struct timespec ts;
  uint64_t ns;

  ptime_clock_gettime(&ts);
  printf("ptime_clock_gettime: %ld sec, %ld ns\n", ts.tv_sec, ts.tv_nsec);
  ns = ptime_to_ns(&ts);
  printf("ptime_to_ns: %"PRIu64" ns\n", ns);
  ns = ptime_gettime_ns();
  printf("ptime_gettime_ns: %"PRIu64" ns\n", ns);
  return 0;
}
