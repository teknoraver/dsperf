#include "model_bandwidth.h"
#include "../timer.h"

double now_in_seconds()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + (tv.tv_usec / 1000000.0);
}

static double get_time_microseconds()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1e6 + ts.tv_nsec / 1000;
}


void run_underlay_bandwidth_client(program_args_t *test, const char *server_ip, int server_port)
{

    size_t block_size = test->block_size;
    size_t mtu = test->mtu_size;
    const char *csv_path = test->csv_path;
    int repetitions = test->repetitions;
    bool formatting_output_csv = test->csv_format;
    bool mtu_defined = test->mtu_defined;
    int mtu_size = test->mtu_size;
    int time = test->time;
    bool time_defined = test->time_defined;

    const uint64_t target_bitrate_bps = 10 * 1000 * 1000; // 10 Mbps

    if (formatting_output_csv)

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
        // printf("\n========== Run %d/%d ==========\n", i + 1, repetitions);

        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0)
        {
            perror("socket");
            return;
        }

        struct sockaddr_in serv_addr = {
            .sin_family = AF_INET,
            .sin_port = htons(server_port),
        };
        inet_pton(AF_INET, server_ip, &serv_addr.sin_addr);

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            perror("connect");
            close(sock);
            continue;
        }

        // Get MSS
        int mss = 0;
        socklen_t optlen = sizeof(mss);
        if(!mtu_defined){
            getsockopt(sock, IPPROTO_TCP, TCP_MAXSEG, &mss, &optlen);
        } else {
            mss = mtu_size - 40;
        }
            
        int chunk_size = (mss < block_size) ? mss : block_size;
        char *buffer = (char *)malloc(chunk_size);
        memset(buffer, 'A', chunk_size);

        int bufsize = 2 * 1024 * 1024; // 2 MB
        setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &bufsize, sizeof(bufsize));
        setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &bufsize, sizeof(bufsize));

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
            ssize_t r = recv(sock, pong_msg, sizeof(pong_msg), 0);
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

        int packet_length = chunk_size; //!!!!!!! negoziato!!!!!!!
        int header_bytes = 40;          // TCP/IP standard header
        int payload_bytes = chunk_size;
        double efficiency = ((double)payload_bytes / (payload_bytes + header_bytes)) * 100.0;
        double packet_to_send = (double)block_size / (double)packet_length;

        if (!formatting_output_csv)
        {
            printf("\n[SUMMARY RUN %d/%d]\n", i + 1, repetitions);
            printf("  Data Block:         %.3f MB\n", block_size / 1.024e6);
            printf("  Protocol:           IPv4\n");
            printf("  Packet Length:      %d bytes\n", packet_length);
            printf("  Header:             %d bytes\n", header_bytes);
            printf("  Efficiency:         %.3f %%\n", efficiency);
            printf("  Pkts to send:     %.3f\n", packet_to_send); // numero pacchetti da inviare
        }

        // printf("[CLIENT] Sending %ld bytes in chunks of %d bytes...\n", block_size, chunk_size);

        int packet_num = 0;
        int total_sent = 0;
        unsigned int interval_us = 0; // Interval between packets (1ms)

        dperf_timer_t *timer = dperf_timer_create(interval_us);
        double start_time = get_time_microseconds();
       
        dperf_timer_start(timer);


        double start_time_def = get_time_microseconds();
        double end_time_def = start_time + time * 1e6; // 'time' è in secondi

        double curr_time = start_time_def;

        while ((time_defined && curr_time < end_time_def) ||
            (!time_defined && total_sent < block_size)) {

            if (!dperf_timer_wait_tick(timer)) {
                break;
            }
            int to_send;

            if(!time_defined){
                to_send = block_size - total_sent;
                if (to_send > chunk_size)
                    to_send = chunk_size;
            }else{
                to_send = chunk_size;
            }

            ssize_t sent = send(sock, buffer, to_send, 0);
            if (sent <= 0)
                break;

            total_sent += sent;
            packet_num++;

            if (time_defined)
                curr_time = get_time_microseconds(); 
        }
        double end_time = get_time_microseconds();
        dperf_timer_stop(timer);
        dperf_timer_destroy(timer);
        
        double elapsed_ms = (end_time - start_time) / 1000; // us -> ms
        double elapsed_s = elapsed_ms / 1000;

        // Dopo aver effettuato il test, posso ottenere il numero di pacchetti persi soltanto ricevendo il numero effettivo che ha raggiunto il server
        // Ovvero il server risponde con il numero effettivo di pacchetti che ha ricevuto!!! ( per testare protocolli stateless ad esempio UDP, nel caso di TCP senza accedere al raw socket!!!)

        double avg_throughput_MBps = ((double)total_sent / (double)1.024e6) / elapsed_s; // / (elapsed_ms / (double)1000); // Ottengo Mega byte per secondo
        double avg_throughput_Mbps = avg_throughput_MBps * (double)8;                      // Ottengo Mega bit per secondo

        double throughput_pps = packet_num / elapsed_s;

        int total_expected = block_size;
        int total_lost = total_expected - total_sent;
        double error_pct = total_expected > 0 ? ((double)total_lost / total_expected) * 100.0 : 0.0;

        /*
        If not global-flags (opzione -y)

        */
        if (!formatting_output_csv)
        {
            printf("  Pkt sent: %d\n", packet_num);                // numero pacchetti inviati
            printf("  Pkt loss: %d\n", 0);                         //
            printf("  Data Sent:         %d bytes\n", total_sent); // Mega bytes
            // printf("  Data Lost:         %d bytes\n", total_lost);
            printf("  Pkt Err. %%:      %.3f %%\n", error_pct);
            printf("  Transfer Time:      %.3f ms\n", elapsed_ms);
            printf("  Throughput:         %.3f MB/s | %.3f Mbps\n", avg_throughput_MBps, avg_throughput_Mbps);
            printf("  Throughput (pps):   %.3f pps\n", throughput_pps);

            printf("[CLIENT] Transfer Time: %.3f ms | Total Bytes: %d | Throughput: %.3f MB/s (%.3f Mbps)\n",
                   elapsed_ms, total_sent, avg_throughput_MBps, avg_throughput_Mbps);
        }
        else
        {
            printf("'%d/%d\t", i + 1, repetitions);
            printf("%.3f\t", block_size / 1.024e6);
            printf("IPv4\t");
            printf("%d\t", packet_length);
            printf("%d\t", header_bytes);
            printf("%.3f\t", efficiency);
            printf("%.3f\t", packet_to_send); // numero pacchetti da inviare
            printf("%d\t", packet_num);       // numero pacchetti inviati
            printf("%d\t", 0);                //
            printf("%.3f\t", ((double)total_sent / (double)1.024e6));       // Mega bytes
            // printf("  Data Lost:         %d bytes\n", total_lost);
            printf("%.3f\t", error_pct);
            printf("%.3f\t", elapsed_ms);
            printf("%.3f\t%.3f\t%.3f\n", avg_throughput_MBps, avg_throughput_Mbps, throughput_pps);
        }

        free(buffer);
        close(sock);
        if (csv_enabled == 1)
            fclose(csv);

        //usleep(1000); // TODO: utilizzare opzione per attivare il ritardo
    }
}
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
//Return a node based on the setup.
DaasAPI* setupNode(daas_setup_t *daas_setup){

    //Instantiate a node and set the callback event
    DaasAPI* node = nullptr;
    auto* dummyHandler = new daasEvent(nullptr);  
    node = new DaasAPI(dummyHandler, "Dperf");
    static_cast<daasEvent*>(dummyHandler)->setNode(node);

    din_t node_din = daas_setup->local_din;
    printf("Local DIN = %i\n", node_din);

    link_setup_t links = daas_setup->links;
    link_t* link_list = links.links;
    char** uri_list = links.uris;
    
    map_setup_t maps = daas_setup->maps;
    din_t* remote_dins_list = maps.remote_dins;
    link_t* remote_link_list = maps.remote_links;
    char** remote_uri_list = maps.uris;

    if(node -> doInit(100, node_din) != ERROR_NONE)
        return nullptr;

    for(int i = 0; i<MAX_LINKS;i++){
        if(link_list[i] != _LINK_NONE && uri_list[i] != nullptr){
            if(node -> enableDriver(link_list[i],uri_list[i]) != ERROR_NONE)
                return nullptr;
        }
    }

    for(int i = 0; i<MAX_REMOTE_LINKS;i++){
        if(remote_link_list[i] != _LINK_NONE && remote_uri_list[i] != nullptr && remote_dins_list[i]){
            if(node -> map(remote_dins_list[i], remote_link_list[i],remote_uri_list[i]) != ERROR_NONE)
                return nullptr;
        }
    }

    if(node -> doPerform(PERFORM_CORE_THREAD) != ERROR_NONE)
        return nullptr;


    return node;

}
void run_overlay_bandwidth_client(daas_setup_t *daas_setup, program_args_t *test){

    int block_size = test->block_size;
    int packet_size = test->packet_size;
    int repetitions = test->repetitions;
    
    DaasAPI* node = setupNode(daas_setup);
    if(node == nullptr) return;

    if(node->listNodes().size() == 0) return;

    //first mapped din for semplicity
    din_t remote_din = node->listNodes()[0];
    printf("Remote Din: %i\n", remote_din);

    if(node-> locate(remote_din)!=ERROR_NONE) return;
    for(int i=0;i<repetitions;i++){
        printf("Sending Test: %i\n", i+1);
        node -> frisbee_dperf(remote_din, 1, block_size, 0);
        usleep(3000000);
    }

    return;

}

void run_overlay_bandwidth_server(daas_setup_t *daas_setup){
     DaasAPI* node = setupNode(daas_setup);
    if(node == nullptr) return;

    if(node->listNodes().size() == 0) return;

    printf("=========== NODE RECEIVER LISTENING....... ================\n");
    while(true){
        usleep(1000);
    }
}
#endif