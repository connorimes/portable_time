#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include "ptime.h"

#define SLEEP_TIME_US 1000000
#define SLEEP_TIME_NS (SLEEP_TIME_US * 1000)

static const uint64_t MAX_DUMMY_ITERS = 1000000;

static void dummy_work(void) {
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
  uint64_t ns1, ns2, elapsed;
  int ret = 0;

  /* Test timing functions */

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


  ns1 = ptime_gettime_ns(PTIME_REALTIME);
  dummy_work();
  errno = 0;
  elapsed = ptime_gettime_elapsed_ns(PTIME_REALTIME, &ns1);
  printf("ptime_gettime_elapsed_ns:PTIME_REALTIME: %"PRIu64" ns\n", elapsed);
  if (elapsed == 0 && errno) {
    perror("ptime_gettime_elapsed_ns:PTIME_REALTIME");
    ret = 1;
  }

  ns1 = ptime_gettime_us(PTIME_MONOTONIC);
  dummy_work();
  errno = 0;
  elapsed = ptime_gettime_elapsed_us(PTIME_MONOTONIC, &ns1);
  printf("ptime_gettime_elapsed_us:PTIME_MONOTONIC: %"PRIu64" us\n", elapsed);
  if (elapsed == 0 && errno) {
    perror("ptime_gettime_elapsed_us:PTIME_MONOTONIC");
    ret = 1;
  }

  /* Test sleeping functions */

  ns1 = ptime_gettime_us(PTIME_MONOTONIC);
  printf("Trying ptime_sleep_ns...\n");
  if (ptime_sleep_ns(SLEEP_TIME_NS)) {
    perror("ptime_sleep_ns");
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
  if (ptime_sleep_us_no_interrupt(SLEEP_TIME_US, NULL)) {
    perror("ptime_sleep_us_no_interrupt");
    ret = 1;
  }
  ns2 = ptime_gettime_us(PTIME_MONOTONIC);
  verify_sleep(SLEEP_TIME_US, (ns2 - ns1), &ret);

  return ret;
}
