#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <time.h>

#include "../dperf_local.hpp"

//DaaS Headers
#ifdef WITH_DAAS
#include "daas.hpp"
#include "daas_types.hpp"
#endif

void run_underlay_bandwidth_client(const char *ip, int port, size_t block_size, size_t mtu, const char *csv_file, int repetitions, bool formatting_output_csv);
void run_underlay_bandwidth_server(int port);

#ifdef WITH_DAAS
void run_overlay_bandwidth_client(daas_setup_t *setup, int blocksize, int packetsize, int repetitions, const char *csv_file,  bool formatting_output_csv);
void run_overlay_bandwidth_server(daas_setup_t *setup);
#endif
