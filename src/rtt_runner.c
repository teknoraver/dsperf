/*
 * DaaS-IoT 2019, 2025 (@) Sebyone Srl
 *
 * File: rtt_runner.c
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

#include "rtt_runner.h"

static double get_time_microseconds()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (ts.tv_sec * 1e6) + (ts.tv_nsec / 1e3);
}

void run_rtt_server(int port, int pkt_size)
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char *buffer = malloc(pkt_size);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_fd, 1);
    printf("[SERVER] Listening on port %d\n", port);

    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
    printf("[SERVER] Connection accepted\n");

    while (1)
    {
        ssize_t rcv = recv(client_fd, buffer, pkt_size, MSG_WAITALL);
        if (rcv <= 0)
            break;
        send(client_fd, buffer, rcv, 0);
    }

    close(client_fd);
    close(server_fd);
    free(buffer);
    printf("[SERVER] Connection closed\n");
}

void run_rtt_client(const char *ip, int port, int pkt_size, int num_pings)
{
    int sock;
    struct sockaddr_in server_addr;
    char *buffer = malloc(pkt_size);
    memset(buffer, 'A', pkt_size);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect");
        exit(1);
    }

    printf("[CLIENT] Connected to %s:%d\n", ip, port);

    double total_bytes = 0;
    double total_time = 0;

    for (int i = 0; i < num_pings; i++)
    {
        double t_start = get_time_microseconds();

        send(sock, buffer, pkt_size, 0);

        ssize_t received = 0;
        while (received < pkt_size)
        {
            ssize_t r = recv(sock, buffer + received, pkt_size - received, 0);
            if (r <= 0)
                break;
            received += r;
        }

        double t_end = get_time_microseconds();
        double rtt_us = t_end - t_start;
        double rtt_s = rtt_us / 1e6;
        double throughput_mbps = (pkt_size * 8.0) / (rtt_s * 1e6); // Megabit/s

        printf("Ping %d | RTT: %.2f us | Throughput: %.2f Mbps\n", i + 1, rtt_us, throughput_mbps);

        total_bytes += pkt_size;
        total_time += rtt_s;
    }

    double avg_throughput = (total_bytes * 8.0) / (total_time * 1e6);
    printf("[CLIENT] Total sent: %.0f bytes | Total time: %.3f s | Avg throughput: %.2f Mbps\n",
           total_bytes, total_time, avg_throughput);

    close(sock);
    free(buffer);
}
