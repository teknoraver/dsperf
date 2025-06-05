#pragma once
#include <ctype.h>
#include <getopt.h>
#include <cstdlib>
#include <cstring>
#include <cstdio>

#ifdef WITH_DAAS
#include "daas.hpp"
#include "daas_types.hpp"
#endif

#define MAX_LINE_LEN 256
#define MAX_LINKS 5
#define MAX_REMOTE_LINKS 64
#define LINK_MAX_VAL 6

#define MIN_BLOCK_SIZE 1
#define MAX_BLOCK_SIZE (1024 * 1024 * 1024) // 1GB
#define MIN_PACKET_SIZE 1
#define MAX_PACKET_SIZE (10 * 1024 * 1024) // 10MB
#define MIN_PORT 1
#define MAX_PORT 65535


//Data Structures for Overlay Setup.


typedef struct {
    int is_sender;            // -1 = unset, 0 = server, 1 = client
    int layer_mode;           // -1 = unset, 0 = underlay, 1 = overlay
    int run_mode;             // -1 = unset, 0 = blocksize, 1 = packet-size
    int block_size;
    int packet_size;
    int repetitions;
    int pings;
    int port;
    char remote_ip[256];
    char overlay_path[256];
    char csv_path[256];

    bool csv_enabled;
    bool version;
} program_args_t;

#ifdef WITH_DAAS
typedef struct {

    link_t links[MAX_LINKS];
    const char* uris[MAX_LINKS];

} link_setup_t;

typedef struct {

    din_t remote_dins[MAX_REMOTE_LINKS];
    link_t remote_links[MAX_REMOTE_LINKS];
    const char* uris[MAX_REMOTE_LINKS];

} map_setup_t;

typedef struct {

    din_t local_din;
    link_setup_t links;
    map_setup_t maps;

} daas_setup_t;


class daasEvent : public IDaasApiEvent {

    public:

    daasEvent(DaasAPI* node) : node_(node) {}
    void dinAcceptedEvent(din_t din) override {}
    void ddoReceivedEvent(int payload_size, typeset_t typeset, din_t din) override {
        DDO *pk;
        if (node_->pull(din, &pk) == ERROR_NONE){
            unsigned char * payload = pk->getPayloadPtr();
            printf("Payload: %s\n", payload);
            delete pk;
        }
    }
    void frisbeeReceivedEvent(din_t din) {}
    void nodeStateReceivedEvent(din_t din) {}
    void atsSyncCompleted(din_t din) {} 
    void frisbeeDperfCompleted(din_t din, uint32_t packets, uint32_t block_size){
        auto dperf_info = node_->get_frisbee_dperf_result();

        double error_pct = packets > 0 ? ((double)(packets - dperf_info.remote_pkt_counter) / packets) * 100.0 : 0.0;
        uint64_t elapsed = dperf_info.remote_last_timestamp - dperf_info.sender_first_timestamp;

        double avg_throughput_MBps = 0.0;
        double avg_throughput_Mbps = 0.0;
        double throughput_pps = 0.0;

        if (elapsed > 0) {
            double elapsed_sec = elapsed / 1000.0;
            avg_throughput_MBps = (dperf_info.remote_data_counter / 1.024e6) / elapsed_sec;
            avg_throughput_Mbps = avg_throughput_MBps * 8;
            throughput_pps = dperf_info.remote_data_counter / elapsed_sec;
        }

        printf("  Pkt sent: %d\n", packets);
        printf("  Pkt loss: %d\n", packets - dperf_info.remote_pkt_counter);
        printf("  Data Sent:        %d bytes\n", block_size);
        printf("  Pkt Err. %%:      %.3f %%\n", error_pct);
        printf("  Transfer Time:    %lld ms\n", elapsed);
        printf("  Throughput:       %.3f MB/s | %.3f Mbps\n", avg_throughput_MBps, avg_throughput_Mbps);
        printf("  Throughput (pps): %.3f pps\n", throughput_pps);
        printf("\n");
        printf("[Node Sender] Transfer Time: %lld ms | Total Bytes: %d | Throughput: %.3f MB/s (%.3f Mbps)\n",
                                elapsed, dperf_info.remote_data_counter,avg_throughput_MBps,avg_throughput_Mbps);

    }

    void setNode(DaasAPI* node) {
        node_ = node;
    }
    private:
    DaasAPI* node_;

};
#endif


// ====================================== PARSING UTIL ===================================================== //

#ifdef WITH_DAAS
link_t parse_link_type(int val);
bool parse_daas_ini(const char* filepath, daas_setup_t *setup);
#endif
void print_usage(const char *prog_name);
void print_options(const char *prog_name);
void parse_args(int argc, char *argv[], program_args_t *args);
int validate_args(program_args_t *args, const char *prog_name);
