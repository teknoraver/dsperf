/*
 * DaaS-IoT 2019, 2025 (@) Sebyone Srl
 *
 * File: block_runner.c
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

#include "block_runner.h"

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
    return (ts.tv_sec * 1e6) + (ts.tv_nsec / 1e3);
}

void run_block_client(const char *server_ip, int server_port, size_t block_size, size_t mtu, const char *csv_path, int repetitions, bool formatting_output_csv, bool csv_no_header)
{

    if (formatting_output_csv && !csv_no_header)
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
        if (getsockopt(sock, IPPROTO_TCP, TCP_MAXSEG, &mss, &optlen) == 0)
        {
            // printf("[CLIENT] MSS negotiated: %d bytes\n", mss);
        }
        /*
        else {
            perror("getsockopt");
            mss = 1448;
        }
        if (mss > 1448) {
            printf("[CLIENT] Warning: MSS (%d) > 1448; using 1448 for safety.\n", mss);
            mss = 1448;
        }
        */

        int chunk_size = (mss < block_size) ? mss : block_size;
        char *buffer = malloc(chunk_size);
        memset(buffer, 'A', chunk_size);

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
        double start_time = get_time_microseconds();
        // double last_time = start_time;

        while (total_sent < block_size)
        {
            int to_send = block_size - total_sent;
            if (to_send > chunk_size)
                to_send = chunk_size;

            ssize_t sent = send(sock, buffer, to_send, 0);
            if (sent <= 0)
                break;

            // double now = get_time_microseconds();
            // double delta_us = now - last_time;
            // double delta_sec = delta_us / 1e6;
            // double throughput_MBps = (sent / 1e6) / (delta_sec > 0 ? delta_sec : 1e-9);
            // printf("[+%.6f s] Sent %zd bytes (%.3f MB/s)\n", (now - start_time) / 1e6, sent, throughput_MBps);
            //  Salvare i dati in memoria e salvare sul file al termine del processo di trasferimento
            /*
            if(csv_enabled==1){
                fprintf(csv, "%.3f,%d,%zd,%.4f,%.6f\n", (now - start_time) / 1000, packet_num, sent, throughput_MBps, rtt_ms);
            }
            */
            total_sent += sent;
            // last_time = now;
            packet_num++;
        }

        double end_time = get_time_microseconds();
        double elapsed_ms = (end_time - start_time) / 1000; // us -> ms

        // Dopo aver effettuato il test, posso ottenere il numero di pacchetti persi soltanto ricevendo il numero effettivo che ha raggiunto il server
        // Ovvero il server risponde con il numero effettivo di pacchetti che ha ricevuto!!! ( per testare protocolli stateless ad esempio UDP, nel caso di TCP senza accedere al raw socket!!!)

        double avg_throughput_MBps = (total_sent / 1.024e6) / (elapsed_ms / 1000); // Ottengo Mega byte per secondo
        double avg_throughput_Mbps = avg_throughput_MBps * 8;                      // Ottengo Mega bit per secondo

        double throughput_pps = packet_num / (elapsed_ms / 1000);

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
            printf("%d\t", total_sent);       // Mega bytes
            // printf("  Data Lost:         %d bytes\n", total_lost);
            printf("%.3f\t", error_pct);
            printf("%.3f\t", elapsed_ms);
            printf("%.3f\t%.3f\t%.3f\n", avg_throughput_MBps, avg_throughput_Mbps, throughput_pps);
        }

        free(buffer);
        close(sock);
        if (csv_enabled == 1)
            fclose(csv);

        //usleep(1000000);
    }
}

void run_block_server(int port)
{
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        perror("socket");
        return;
    }

    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr.s_addr = INADDR_ANY,
    };

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

        char *buffer = malloc(mss > 0 ? mss : 2048);
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
