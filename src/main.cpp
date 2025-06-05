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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <getopt.h>
#include <stdbool.h>
#include "version.h"


// daas
// #include "daas.hpp"
// #include "daas_types.hpp"


#if defined(__linux__) || defined(__RASP__)
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/time.h>
#endif

//Include test modules
#include "system_info.h"
#include "test_models/model_bandwidth.h"
#include "dperf_local.hpp"
#include "rtt_runner.h"

// Constants for configuration



void start_underlay_server(program_args_t *test);
void start_underlay_client(program_args_t *test);

#ifdef WITH_DAAS
void start_daas_client(daas_setup_t *setup, program_args_t *test);
void start_daas_server(daas_setup_t *setup);
#endif

double now_sec()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1e6;
}


void start_underlay_server(program_args_t *test)
{
    switch (test->run_mode)
    {
    case 0:
    {
        run_underlay_bandwidth_server(test->port);
        break;
    }
    case 1:
    {
        run_rtt_server(test->port, test->packet_size);
        break;
    }
    }
}

void start_underlay_client(program_args_t *test)
{
    char ip[64];
    int port;
    const char* host = test->remote_ip;
    const char *colon = strchr(host, ':');
    if (colon == NULL)
    {
        fprintf(stderr, "[CLIENT] Invalid host format. Use IP:PORT\n");
        return;
    }

    size_t ip_len = colon - host;
    strncpy(ip, host, ip_len);
    ip[ip_len] = '\0';
    port = atoi(colon + 1);

    if (port < MIN_PORT || port > MAX_PORT)
    {
        fprintf(stderr, "[CLIENT] Invalid port number: %d\n", port);
        return;
    }

    if (!test->csv_format)
    {
        printf("dperf started as client to %s:%d with %s size %d\n",
               ip, port, (test->block_size > 0) ? "block" : "packet",
               (test->block_size > 0) ? test->block_size : test->packet_size);
    }
    switch (test->run_mode)
    {
    case 0:
    {
        run_underlay_bandwidth_client(test, ip, port);
        break;
    }
    case 1:
    {
        run_rtt_client(ip, port, test->packet_size, test->pings);
        break;
    }
    }
}
#ifdef WITH_DAAS
void start_daas_server(daas_setup_t *setup)
{

    run_overlay_bandwidth_server(setup);

}

void start_daas_client(daas_setup_t *setup, program_args_t *test)
{

    run_overlay_bandwidth_client(setup, test->block_size, test->packet_size , test->repetitions, test->csv_path, test->csv_format);

    printf("Daas not included\n");

}
#endif



int main(int argc, char *argv[])
{

    program_args_t args;
#ifdef WITH_DAAS
    daas_setup_t daas_setup;
#endif
    parse_args(argc, argv, &args);

    if (validate_args(&args, argv[0]) != EXIT_SUCCESS) {
        return EXIT_FAILURE;
    }
#ifdef WITH_DAAS
     if (args.layer_mode == 1) {
        if (!parse_daas_ini(args.overlay_path, &daas_setup)) {
            fprintf(stderr, "Error: Failed to parse .ini overlay file.\n");
            return EXIT_FAILURE;
        }
    }
#endif
    if (args.csv_enabled)
{
    FILE *f = fopen(args.csv_path, "w");
    if (f == NULL)
    {
        perror("Error opening CSV file");
        return EXIT_FAILURE;
    }

    time_t now = time(NULL);
    char date_str[64];
    strftime(date_str, sizeof(date_str), "%Y-%m-%d %H:%M:%S", localtime(&now));

    fprintf(f, "Host: %s\n", get_os_name());
    fprintf(f, "Architecture: %s\n", get_architecture());
    fprintf(f, "Kernel: %s\n", get_kernel_version());
    fprintf(f, "Date-Time: %s\n", date_str);

    if (args.block_size > 0)
    {
        fprintf(f, "Data Block Size: %d\n", args.block_size);
    }
    else
    {
        fprintf(f, "Packet Size: %d\n", args.packet_size);
        fprintf(f, "Packet Count: %d\n", args.pings);
    }

    fprintf(f, "Layer: %s\n", args.layer_mode == 0 ? "Underlay" : "DaaS Overlay");
    fprintf(f, "Layer Version: %s\n", args.layer_mode == 0 ? "IPv4 plain sockets" : "DaaS Layer");

    fclose(f);
}

    if (args.layer_mode == 0)
    {
        if (args.is_sender)
        {
            start_underlay_client(&args);
        }
        else
        {
            start_underlay_server(&args);
        }
    }
    else
    {
#ifdef WITH_DAAS
        if (args.is_sender)
        {

            start_daas_client(&daas_setup, &args);
        }
        else
        {
            start_daas_server(&daas_setup);
        }
#else
	printf("Daas not included\n");
#endif
    }

    return EXIT_SUCCESS;
}