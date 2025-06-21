/*
 * DaaS-IoT 2019, 2025 (@) Sebyone Srl
 *
 * File: loopback.c
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.
 *
 * Disclaimer of Warrant
 * Covered Software is provided under this License on an "as is" basis, without warranty of any kind, either
 * expressed, implied, or statutory, including, without limitation, warranties that the Covered  Software is
 * free of defects, merchantable, fit for a particular purpose or non-infringing.
 * The entire risk as to the quality and performance of the Covered Software is with You.  Should any Covered
 * Software prove defective in any respect, You (not any Contributor) assume the cost of any necessary
 * servicing, repair, or correction.
 * This disclaimer of warranty constitutes an essential part of this License.  No use of any Covered Software
 * is authorized under this License except under this disclaimer.
 *
 * Limitation of Liability
 * Under no circumstances and under no legal theory, whether tort (including negligence), contract, or otherwise,
 * shall any Contributor, or anyone who distributes Covered Software as permitted above, be liable to You for
 * any direct, indirect, special, incidental, or consequential damages of any character including, without
 * limitation, damages for lost profits, loss of goodwill, work stoppage, computer failure or malfunction,
 * or any and all other commercial damages or losses, even if such party shall have been informed of the
 * possibility of such damages.  This limitation of liability shall not apply to liability for death or personal
 * injury resulting from such party's negligence to the extent applicable law prohibits such limitation.
 * Some jurisdictions do not allow the exclusion or limitation of incidental or consequential damages, so this
 * exclusion and limitation may not apply to You.
 *
 * Contributors:
 * plogiacco@smartlab.it - initial design, implementation and documentation
 * sebastiano.meduri@gmail.com  - initial design, implementation and documentation
 *
 */

#ifndef TIMER_H
#define TIMER_H

#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // Timer struct
    typedef struct
    {
        unsigned int interval_us;
        pthread_t thread;
        pthread_mutex_t mutex;
        pthread_cond_t cond;
        bool running;
        bool tick;
    } dsperf_timer_t;

    // Create a timer that "wakes up" in an defined interval
    dsperf_timer_t *dsperf_timer_create(unsigned int interval_us);

    // Destroy timer and resource
    void dsperf_timer_destroy(dsperf_timer_t *t);

    // Start timer in an internal thread
    void dsperf_timer_start(dsperf_timer_t *t);

    // Stop timer (wait for the internal thread)
    void dsperf_timer_stop(dsperf_timer_t *t);

    // the client calls this function to wait for the time
    // returns true if timer is active, false otherwise
    bool dsperf_timer_wait_tick(dsperf_timer_t *t);

#ifdef __cplusplus
}
#endif

#endif
