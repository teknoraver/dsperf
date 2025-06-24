#include "model_bandwidth.h"
#include "../timer.h"

#define MODEL_NAME "IPv4/TCP"                  // sub 6 (TCP)
#define _Byte2Megabyte(b) ((double)(b) / (1 << 20))

// Sub Protocol as defined in RFC791
//
// Numero di protocollo	Nome del protocollo	Abbreviazione
// ----+---------------------------------------+----------
// 1	Internet Control Message Protocol	    ICMP
// 2	Internet Group Management Protocol	    IGMP
// 6	Transmission Control Protocol	        TCP
// 17	User Datagram Protocol	                UDP
// 41	IPv6 encapsulation	                    ENCAP
// 89	Open Shortest Path First	            OSPF
// 132	Stream Control Transmission Protocol	SCTP

double now_in_seconds()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + (ts.tv_nsec / 1e9);
}

static double get_time_microseconds()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e6 + ts.tv_nsec / 1000;
}

// Sends a block of data to remote server
// sums all sended bytes and computes the indexes
//
// Data Block [MB]
// Protocol - IPV4 current test model
// Pkt Length [bytes]
// Header [bytes]   - 17 bytes IP header + sub-protocol options 0..34 bytes
//
// Efficiency [%]   - ratio: [%] = payload / total_packet_size ( header+payload )
// Pkts to send   - trunc(blocksize / packet_payload) + (blocksize % packet_payload)
// Pkt sent       - counter of packet really sended ( check socket buffering settings !!!!!!!)
// Pkt loss         - ???????????????????????
// Data Sent [MB]  -
// Pkt Err.[%]
// Transfer Time [ms]
// Throughput [MBps]  [Mbps] [pps]

// --------------------------------------------------------------------------------------------------------
void run_underlay_bandwidth_client(program_args_t *_test, const char *_server_ip, int _server_port)
{
    size_t block_size = _test->block_size;
    size_t mtu = _test->mtu_size;
    const char *csv_path = _test->csv_path;
    int repetitions = _test->repetitions;
    bool __flag_output_csv = _test->csv_format;
    bool csv_no_header = _test->csv_no_header;
    bool mtu_defined = _test->mtu_defined;
    int mtu_size = _test->mtu_size;
    int time = _test->time;
    bool __flag_time_defined = _test->time_defined;

    const uint64_t target_bitrate_bps = 10 * 1000 * 1000; // 10 Mbps (Megabits)

    if (__flag_output_csv && csv_no_header)
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
    }

    for (int i = 0; i < repetitions; i++)
    {

        int mss = 0; // packet's payload size
        int pkt_payload_size;
        int pkt_header_size = 40; // TCP/IP standard header
        char *pkt_payload;
        double efficiency;
        double pkts_to_send;

        int sock = socket(AF_INET, SOCK_STREAM, 0);

        if (sock < 0)
        {
            perror("socket");
            return;
        }

        struct sockaddr_in serv_addr = {
            .sin_family = AF_INET,
            .sin_port = htons(_server_port),
        };
        inet_pton(AF_INET, _server_ip, &serv_addr.sin_addr);

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            perror("connect");
            close(sock);
            continue;
        }

        // Get socket MSS settings

        socklen_t optlen = sizeof(mss);
        if (!mtu_defined)
        {
            getsockopt(sock, IPPROTO_TCP, TCP_MAXSEG, &mss, &optlen);
        }
        else
        {
            mss = mtu_size - 40;
        }

        // Sets send and receive socket buffers size
        int bufsize = 2 * 1024 * 1024;                                      // 2 MB
        setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize)); // Send !!!!!!!
        setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));

        /* Solo se flag selezionato !!!!!!!
        FILE *csv = fopen(csv_path, i == 0 ? "w" : "a");
        int csv_enabled = 1;
        if (!csv)
        {
            csv_enabled = 0;
        }

        if (i == 0 && csv_enabled == 1)
            fprintf(csv, "Timestamp_ms,Num Pkt,Dimensione Pkt,Throughput_MBps,RTT_ms\n");



        char ping_msg[] = "PING";
        char pong_msg[5] = {0};

        double rtt_ms = 0;

        double ping_start = get_time_microseconds();

        if (send(sock, ping_msg, sizeof(ping_msg), 0) != sizeof(ping_msg))
        {
            perror("[CLIENT] Error sending PING");
        }
        else
        {
            ssize_t r = recv(sock, pong_msg, sizeof(pong_msg), 0);  // Bloccante !!!!!!!!!!

            if (r != sizeof(pong_msg) || strcmp(pong_msg, "PONG") != 0)
            {
                fprintf(stderr, "[CLIENT] Invalid PONG response\n");
            }
            else
            {
                double ping_end = get_time_microseconds();
                rtt_ms = (ping_end - ping_start) / 1000.0;
                // printf("[CLIENT] RTT misurato: %.3f ms\n", rtt_ms);
            }
        }
        */

        /// computes number of packet will be sended

        pkt_payload_size = (mss < block_size) ? mss : block_size;
        pkt_payload = (char *)malloc(pkt_payload_size);
        memset(pkt_payload, 'A', pkt_payload_size);

        //    int packet_length = pkt_payload_size; //!!!!!!! negoziato!!!!!!!
        //    int payload_bytes = pkt_payload_size;

        efficiency = ((double)pkt_payload_size / (pkt_payload_size + pkt_header_size)) * 100.0;
        pkts_to_send = (double)block_size / (double)pkt_payload_size;

        if (!__flag_output_csv)
        {
            printf("\n[SUMMARY RUN %d/%d]\n", i + 1, repetitions);
            printf("  Data Block:         %.3f MB\n", _Byte2Megabyte(block_size));
            printf("  Protocol:           IPv4\n");
            printf("  Packet Length:      %d bytes\n", pkt_payload_size);
            printf("  Header:             %d bytes\n", pkt_header_size);
            printf("  Efficiency:         %.3f %%\n", efficiency);
            printf("  Pkts to send:     %.3f\n", pkts_to_send); // numero pacchetti da inviare
        }

        // printf("[CLIENT] Sending %ld bytes in chunks of %d bytes...\n", block_size, chunk_size);

        int pkts_counter = 0;
        int bytes_sent = 0;

        unsigned int interval_us = 0; // Interval between packets (1ms)

        dsperf_timer_t *timer = dsperf_timer_create(interval_us);
        double start_time = get_time_microseconds();

        dsperf_timer_start(timer);

        double start_time_def = get_time_microseconds();
        double end_time_def = start_time + time * 1e6; // 'time' è in secondi

        double curr_time = start_time_def;

        ssize_t sent;

        // Main sending cycle !!!!!!

        int to_send;

        if (__flag_time_defined) // Runs in time-mode
        {
            // curr_time < end_time_def
            while ((curr_time < end_time_def))
            {

                to_send = pkt_payload_size;
                if (send(sock, pkt_payload, to_send, 0) > 0)
                    break;
                curr_time = get_time_microseconds();
            }
        }
        else // runs in block-size mode
        {
            while ((bytes_sent < block_size))
            {

                if (!dsperf_timer_wait_tick(timer)) // Semaphore
                {
                    break;
                }
                to_send = block_size - bytes_sent;
                if (to_send > pkt_payload_size)
                    to_send = pkt_payload_size;

                // to_send = pkt_payload_size;
                if ((sent = send(sock, pkt_payload, pkt_payload_size, 0)) <= 0)
                    break;

                bytes_sent += sent;
                pkts_counter++;
            }
        }

        double end_time = get_time_microseconds();

        dsperf_timer_stop(timer);
        dsperf_timer_destroy(timer);

        double elapsed_ms = (end_time - start_time) / 1000; // us -> ms
        double elapsed_s = elapsed_ms / 1000;

        // Dopo aver effettuato il test, posso ottenere il numero di pacchetti persi soltanto ricevendo il numero effettivo che ha raggiunto il server
        // Ovvero il server risponde con il numero effettivo di pacchetti che ha ricevuto!!! ( per testare protocolli stateless ad esempio UDP, nel caso di TCP senza accedere al raw socket!!!)

        double out_throughput_Mbytes = (_Byte2Megabyte(bytes_sent) / (double)elapsed_s); // / (elapsed_ms / (double)1000); // Ottengo Mega byte per secondo
        double out_throughput_Mbits = out_throughput_Mbytes * (double)8;                 // Ottengo Mega bit per secondo
        double out_throughput_pps = (double)pkts_counter / elapsed_s;

        int out_bytes_to_send = block_size; // Total byte to send
        int out_bytes_lost = out_bytes_to_send - bytes_sent;

        double out_bytes_lost_percents = out_bytes_to_send > 0 ? ((double)out_bytes_lost / out_bytes_to_send) * 100.0 : 0.0;

        if (!__flag_output_csv)
        {
            printf("  Pkt sent: %d\n", pkts_counter);              // numero pacchetti inviati
            printf("  Pkt loss: %d\n", 0);                         //
            printf("  Data Sent:         %d bytes\n", bytes_sent); // Mega bytes
            // printf("  Data Lost:         %d bytes\n", total_lost);
            printf("  Pkt Err. %%:      %.3f %%\n", out_bytes_lost_percents);
            printf("  Transfer Time:      %.3f ms\n", elapsed_ms);
            printf("  Throughput:         %.3f MB/s | %.3f Mbps\n", out_throughput_Mbytes, out_throughput_Mbits);
            printf("  Throughput (pps):   %.3f pps\n", out_throughput_pps);

            printf("[CLIENT] Transfer Time: %.3f ms | Total Bytes: %d | Throughput: %.3f MB/s (%.3f Mbps)\n",
                   elapsed_ms, bytes_sent, out_throughput_Mbytes, out_throughput_Mbits);
        }
        else
        {
            printf("'%d/%d\t", i + 1, repetitions);
            printf("%.3f\t", _Byte2Megabyte(block_size));
            printf("IPv4\t");
            printf("%d\t", pkt_payload_size);
            printf("%d\t", pkt_header_size);
            printf("%.3f\t", efficiency);
            printf("%.3f\t", pkts_to_send);               // numero pacchetti da inviare
            printf("%d\t", pkts_counter);                 // numero pacchetti inviati
            printf("%d\t", 0);                            //
            printf("%.3f\t", _Byte2Megabyte(bytes_sent)); // Mega bytes
            // printf("  Data Lost:         %d bytes\n", total_lost);
            printf("%.3f\t", out_bytes_lost_percents);
            printf("%.3f\t", elapsed_ms);
            printf("%.3f\t%.3f\t%.3f\n", out_throughput_Mbytes, out_throughput_Mbits, out_throughput_pps);
        }

        free(pkt_payload);
        close(sock);
        /*
        if (csv_enabled == 1)
            fclose(csv);
        */
        // usleep(1000); // TODO: utilizzare opzione per attivare il ritardo
    }
}


// --------------------------------------------------------------------------------------------------------
void run_underlay_bandwidth_server(int port)
{
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        perror("socket");
        return;
    }

    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        close(server_sock);
        return;
    }

    if (listen(server_sock, 1) < 0)
    {
        perror("listen");
        close(server_sock);
        return;
    }

    printf("[SERVER] In ascolto sulla porta %d...\n", port);

    int run_count = 0;
    while (1)
    {
        run_count++;
        printf("\n========== Waiting for connection %d (port %d) ==========\n", run_count, port);

        int client_sock = accept(server_sock, NULL, NULL);
        if (client_sock < 0)
        {
            perror("accept");
            continue;
        }

        int mss = 0;
        socklen_t optlen = sizeof(mss);
        if (getsockopt(client_sock, IPPROTO_TCP, TCP_MAXSEG, &mss, &optlen) != 0)
        {
            // printf("[SERVER] MSS negotiated: %d bytes\n", mss);
            perror("not load mss");
        }

        char *buffer = (char *)malloc(mss > 0 ? mss : 2048);
        if (!buffer)
        {
            perror("[SERVER] malloc failed");
            close(client_sock);
            continue;
        }

        int total_received = 0;
        double start = get_time_microseconds();

        /*
        // Scambio RTT
        char ping_buf[5] = {0};
        ssize_t ping_len = recv(client_sock, ping_buf, sizeof(ping_buf), 0);
        if (ping_len == sizeof(ping_buf) && strcmp(ping_buf, "PING") == 0)
        {
            if (send(client_sock, "PONG", 5, 0) != 5)
            {
                perror("[SERVER] Errore invio PONG");
                free(buffer);
                close(client_sock);
                continue;
            }
        }
        else
        {
            fprintf(stderr, "[SERVER] Ping non ricevuto correttamente\n");
            free(buffer);
            close(client_sock);
            continue;
        }
        */

        while (1)
        {
            ssize_t recvd = recv(client_sock, buffer, mss > 0 ? mss : 2048, 0);
            if (recvd <= 0)
                break;
            total_received += recvd;
        }

        double end = get_time_microseconds();
        double seconds = (end - start) / 1e6;
        double throughput = (seconds > 0) ? (total_received / (1024.0 * 1024.0) / seconds) : 0;

        printf("[SERVER] Run %d - Time: %.6f s | Bytes: %d | Throughput: %.3f MB/s\n",
               run_count, seconds, total_received, throughput);

        free(buffer);
        close(client_sock);
    }

    close(server_sock);
}


#ifdef WITH_DAAS
// --------------------------------------------------------------------------------------------------------
// Return a node based on the setup.
DaasAPI *setupNode(daas_setup_t *daas_setup, bool csv_format, bool csv_no_header)
{

    // Instantiate a node and set the callback event
    DaasAPI *node = nullptr;
    auto *dummyHandler = new daasEvent(nullptr, csv_format, csv_no_header);
    node = new DaasAPI(dummyHandler, "Dperf");
    static_cast<daasEvent *>(dummyHandler)->setNode(node);

    din_t node_din = daas_setup->local_din;
    printf("Local DIN = %i\n", node_din);

    link_setup_t links = daas_setup->links;
    link_t *link_list = links.links;
    char **uri_list = links.uris;

    map_setup_t maps = daas_setup->maps;
    din_t *remote_dins_list = maps.remote_dins;
    link_t *remote_link_list = maps.remote_links;
    char **remote_uri_list = maps.uris;

    if (node->doInit(100, node_din) != ERROR_NONE)
        return nullptr;

    for (int i = 0; i < MAX_LINKS; i++)
    {
        if (link_list[i] != _LINK_NONE && uri_list[i] != nullptr)
        {
            if (node->enableDriver(link_list[i], uri_list[i]) != ERROR_NONE)
                return nullptr;
        }
    }

    for (int i = 0; i < MAX_REMOTE_LINKS; i++)
    {
        if (remote_link_list[i] != _LINK_NONE && remote_uri_list[i] != nullptr && remote_dins_list[i])
        {
            if (node->map(remote_dins_list[i], remote_link_list[i], remote_uri_list[i]) != ERROR_NONE)
                return nullptr;
        }
    }

    if (node->doPerform(PERFORM_CORE_THREAD) != ERROR_NONE)
        return nullptr;

    return node;
}

// --------------------------------------------------------------------------------------------------------
void run_overlay_bandwidth_client(daas_setup_t *daas_setup, program_args_t *test)
{

    int block_size = test->block_size;
    int repetitions = test->repetitions;
    bool csv_format = test->csv_format;
    bool csv_no_header = test->csv_no_header;
    int pack_num = test->pack_num;

    DaasAPI *node = setupNode(daas_setup, csv_format, csv_no_header);
    if (node == nullptr)
        return;

    if (node->listNodes().size() == 0)
        return;

    // first mapped din for semplicity
    /*
    din_t remote_din = node->listNodes()[0];
    printf("Remote Din: %i\n", remote_din);
    */
    din_t remote_din = test->remote_din;

    if (node->locate(remote_din) != ERROR_NONE)
        return;
    for (int i = 0; i < repetitions; i++)
    {
        printf("Sending Test: %i\n", i + 1);
        node->frisbee_dperf(remote_din, pack_num, block_size, 0);
        usleep(1000000);
    }

    return;
}

// --------------------------------------------------------------------------------------------------------
void run_overlay_bandwidth_server(daas_setup_t *daas_setup)
{
    DaasAPI *node = setupNode(daas_setup, false, false);
    if (node == nullptr)
        return;

    if (node->listNodes().size() == 0)
        return;

    printf("=========== NODE RECEIVER LISTENING....... ================\n");
    while (true)
    {
        usleep(1000);
    }
}


#endif // WITH_DAAS
