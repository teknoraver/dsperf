#ifndef TIMER_H
#define TIMER_H

#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// Timer struct
typedef struct {
    unsigned int interval_us;
    pthread_t thread;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    bool running;
    bool tick;
} dperf_timer_t;

// Create a timer that "wakes up" in an defined interval
dperf_timer_t *dperf_timer_create(unsigned int interval_us);

// Destroy timer and resource
void dperf_timer_destroy(dperf_timer_t *t);

// Start timer in an internal thread
void dperf_timer_start(dperf_timer_t *t);

// Stop timer (wait for the internal thread)
void dperf_timer_stop(dperf_timer_t *t);

// the client calls this function to wait for the time
// returns true if timer is active, false otherwise
bool dperf_timer_wait_tick(dperf_timer_t *t);

#ifdef __cplusplus
}
#endif

#endif
