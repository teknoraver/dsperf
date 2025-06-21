#define _POSIX_C_SOURCE 200809L
#include "timer.h"


static void *dsperf_timer_thread_func(void *arg) {
    dsperf_timer_t *t = (dsperf_timer_t *)arg;

    struct timespec ts;
    while (1) {
        // timenow + interval
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += t->interval_us / 1000000;
        ts.tv_nsec += (t->interval_us % 1000000) * 1000;
        if (ts.tv_nsec >= 1000000000) {
            ts.tv_sec++;
            ts.tv_nsec -= 1000000000;
        }

        pthread_mutex_lock(&t->mutex);
        if (!t->running) {
            pthread_mutex_unlock(&t->mutex);
            break;
        }

        // tick signal
        t->tick = true;
        pthread_cond_broadcast(&t->cond);

        // wait for timeout or stops
        int rc = pthread_cond_timedwait(&t->cond, &t->mutex, &ts);
        if (!t->running) {
            pthread_mutex_unlock(&t->mutex);
            break;
        }
        pthread_mutex_unlock(&t->mutex);
    }
    return NULL;
}

dsperf_timer_t *dsperf_timer_create(unsigned int interval_us) {
    dsperf_timer_t *t = malloc(sizeof(dsperf_timer_t));
    if (!t)
        return NULL;

    t->interval_us = interval_us;
    t->running = false;
    t->tick = false;
    pthread_mutex_init(&t->mutex, NULL);
    pthread_cond_init(&t->cond, NULL);
    return t;
}

void dsperf_timer_destroy(dsperf_timer_t *t) {
    if (!t) return;
    dsperf_timer_stop(t);
    pthread_mutex_destroy(&t->mutex);
    pthread_cond_destroy(&t->cond);
    free(t);
}

void dsperf_timer_start(dsperf_timer_t *t) {
    if (!t) return;
    pthread_mutex_lock(&t->mutex);
    t->running = true;
    pthread_mutex_unlock(&t->mutex);
    pthread_create(&t->thread, NULL, dsperf_timer_thread_func, t);
}

void dsperf_timer_stop(dsperf_timer_t *t) {
    if (!t) return;
    pthread_mutex_lock(&t->mutex);
    t->running = false;
    pthread_cond_broadcast(&t->cond);
    pthread_mutex_unlock(&t->mutex);
    pthread_join(t->thread, NULL);
}

bool dsperf_timer_wait_tick(dsperf_timer_t *t) {
    if (!t) return false;

    pthread_mutex_lock(&t->mutex);
    while (!t->tick && t->running) {
        pthread_cond_wait(&t->cond, &t->mutex);
    }
    if (!t->running) {
        pthread_mutex_unlock(&t->mutex);
        return false;
    }
    t->tick = false; // reset
    pthread_mutex_unlock(&t->mutex);
    return true;
}
