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
    int layer_mode;           // -1 = unset, 0 = underlay, 1 = daas
    int block_size;           // dimensione del blocco (solo client)
    int mtu_size;
    bool mtu_defined;
    int repetitions;          // solo client, default 1
    int pack_num;
    int port;                 // server con underlay: porta di ascolto
    char remote_ip[256];      // client con underlay: IP:PORT stringa
    int remote_din;           // client/server con daas: remote DIN (intero)
    char overlay_path[256];   // percorso file ini per daas
    char csv_path[256];
    bool csv_enabled;
    bool csv_format;
    bool csv_no_header;
    int time;
    bool time_defined;
    bool version;
} program_args_t;

#ifdef WITH_DAAS
typedef struct {

    link_t links[MAX_LINKS];
    char* uris[MAX_LINKS];

} link_setup_t;

typedef struct {

    din_t remote_dins[MAX_REMOTE_LINKS];
    link_t remote_links[MAX_REMOTE_LINKS];
    char* uris[MAX_REMOTE_LINKS];

} map_setup_t;

typedef struct {

    din_t local_din;
    link_setup_t links;
    map_setup_t maps;

} daas_setup_t;


class daasEvent : public IDaasApiEvent {

    public:

    daasEvent(DaasAPI* node, bool csv_format, bool csv_no_header) : node_(node), csv_format_(csv_format), csv_no_header_(csv_no_header) {}
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

        if (csv_format_ && csv_no_header_)
    {
        printf("#/#\t");
        printf("Data Block [MB]\t");
        printf("Protocol\t");
        printf("Pkt Length [bytes]\t");
        printf("Header [bytes]\t");
        printf("Efficiency[%%]\t");
        printf("Pkts to send\t");  // numero pacchetti da inviare
        printf("Pkt sent\t");      // numero pacchetti inviati
        printf("Pkt loss\t");      //
        printf("Data Sent[MB]\t"); // Mega bytes
        printf("Pkt Err.[%%]\t");
        printf("Transfer Time [ms]\t");
        printf("Throughput [MB/s]\t[Mb/s]\t[pps]\n");
        printf("Sender first timestamp\t");
        printf("Local end timestamp\t");
        printf("Remote first timestamp\t");
        printf("Remote last timestamp\t");
    }


        double error_pct = packets > 0 ? ((double)(packets - dperf_info.remote_pkt_counter) / packets) * 100.0 : 0.0;
        uint64_t elapsed = dperf_info.remote_last_timestamp - dperf_info.sender_first_timestamp;

        double throughput_MBps = 0.0;
        double throughput_Mbps = 0.0;
        double throughput_pps = 0.0;

        if (elapsed > 0) {
            double elapsed_sec = elapsed / 1000.0;
            throughput_MBps = (dperf_info.remote_data_counter / 1.024e6) / elapsed_sec;
            throughput_Mbps = throughput_MBps * 8;
            throughput_pps = dperf_info.remote_data_counter / elapsed_sec;
        }

        if (!csv_format_) {
        printf("  Pkt sent: %d\n", packets);
        printf("  Pkt loss: %d\n", packets - dperf_info.remote_pkt_counter);
        printf("  Data Sent:        %d bytes\n", block_size);
        printf("  Pkt Err. %%:      %.3f %%\n", error_pct);
        printf("  Transfer Time:    %llu ms\n", (unsigned long long)elapsed);
        printf("  Throughput:       %.3f MB/s | %.3f Mbps\n", throughput_MBps, throughput_Mbps);
        printf("  Throughput (pps): %.3f pps\n", throughput_pps);
        printf("\n");
        printf("[Node Sender] Transfer Time: %llu ms | Total Bytes: %d | Throughput: %.3f MB/s (%.3f Mbps)\n",
               (unsigned long long)elapsed, dperf_info.remote_data_counter, throughput_MBps, throughput_Mbps);
        printf("[Local First timestamp]=%.3f | [Local End timestamp]=%.3f | [Remote First timestamp]=%.3f | [Remote Last timestamp]=%.3f\n",
             (double)dperf_info.sender_first_timestamp, (double)dperf_info.local_end_timestamp, (double)dperf_info.remote_first_timestamp, (double)dperf_info.remote_last_timestamp);
    } else {
        printf("%d\t", 1);  // es: Run number o id (adatta come vuoi)
        printf("%.3f\t", (double)block_size / 1.024e6); // Data Block in MB
        printf("IPv4\t");  // Protocollo fisso o variabile se serve
        printf("%d\t", dperf_info.remote_pkt_counter > 0 ? (block_size / dperf_info.remote_pkt_counter) : 0);  // Packet Length approx
        printf("40\t"); // Header bytes fisso o calcolato
        double efficiency = ((double)block_size / (block_size + 40 * dperf_info.remote_pkt_counter)) * 100.0;
        printf("%.3f\t", efficiency);
        printf("%.3f\t", (double)block_size / (block_size / (dperf_info.remote_pkt_counter > 0 ? dperf_info.remote_pkt_counter : 1))); // Pkts to send (approx)
        printf("%d\t", dperf_info.remote_pkt_counter);  // Pkt sent
        printf("%d\t", packets - dperf_info.remote_pkt_counter); // Pkt loss
        printf("%.3f\t", (double)dperf_info.remote_data_counter / 1.024e6); // Data sent MB
        printf("%.3f\t", error_pct);
        printf("%llu\t", (unsigned long long)elapsed);
        printf("%.3f\t%.3f\t%.3f\n", throughput_MBps, throughput_Mbps, throughput_pps);
        printf("%.3f\t", (double)dperf_info.sender_first_timestamp);
        printf("%.3f\t", (double)dperf_info.local_end_timestamp);
        printf("%.3f\t", (double)dperf_info.remote_first_timestamp);
        printf("%.3f\t", (double)dperf_info.remote_last_timestamp);
    } 
    }

    void setNode(DaasAPI* node) {
        node_ = node;
    }
    private:
    DaasAPI* node_;
    bool csv_format_;
    bool csv_no_header_;

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
