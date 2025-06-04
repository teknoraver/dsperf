/*
 * DaaS-IoT 2019, 2025 (@) Sebyone Srl
 *
 * File: system_info.h
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
 * l.grillo@sebyone.com  - implementation and documentation
 * psestito@sebyone.it - implementation and documentation
 * mpagano@sebyone.it - implementation and documentation
 * 
 */

#ifndef SYSTEM_INFO_H
#define SYSTEM_INFO_H

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------
// HOST
// -----------------------

const char* get_os_name();
const char* get_architecture();
const char* get_kernel_version();



// ------------------------
// CPU INFO
// ------------------------

// Ottiene il modello della CPU (stringa). 'buffer' deve essere abbastanza grande.
void get_cpu_model(char *buffer, unsigned int size);

// Ottiene il numero di core logici (ritorna -1 in caso di errore)
int get_cpu_cores();

// Ritorna la temperatura della CPU in gradi Celsius, -1.0f se non disponibile
float get_cpu_temperature();


// ------------------------
// MEMORIA
// ------------------------

// Ottiene il valore (in KB) associato a una label in /proc/meminfo
long get_meminfo_kb(const char *label);

// Ottiene memoria totale, libera e usata
int get_memory_usage(long *total, long *free, long *used);

// ------------------------
// LOAD AVERAGE
// ------------------------

// Ottiene il load average a 1, 5 e 15 minuti
int get_load_average(float *avg1, float *avg5, float *avg15);

// ------------------------
// RETE (NETWORK)
// ------------------------

#define MAX_INTERFACES 32
#define MAX_IFACE_NAME 32

// Struct per dettagli uso rete
typedef struct {
    long rx_bytes;
    long rx_packets;
    long rx_errs;
    long rx_drop;
    long rx_fifo;
    long rx_frame;
    long rx_compressed;
    long rx_multicast;

    long tx_bytes;
    long tx_packets;
    long tx_errs;
    long tx_drop;
    long tx_fifo;
    long tx_colls;
    long tx_carrier;
    long tx_compressed;
} net_stats_t;

// Ottiene i byte ricevuti e trasmessi da una singola interfaccia
int get_net_usage(const char *iface, long *rx_bytes, long *tx_bytes);

// Ottiene info dettagliate rete per interfaccia specifica
int get_net_usage_detailed(const char *iface, net_stats_t *stats);

// Ottiene l'elenco delle interfacce disponibili
// interfaces è un array 2D: interfaces[MAX_INTERFACES][MAX_IFACE_NAME]
// Ritorna il numero di interfacce trovate, -1 in caso di errore
int get_network_interfaces(char interfaces[MAX_INTERFACES][MAX_IFACE_NAME]);

// Calcola il totale di byte ricevuti e trasmessi su tutte le interfacce
// (fa un for interno sulle interfacce)
int get_total_net_usage(long *total_rx, long *total_tx);

// ------------------------
// DISCO
// ------------------------

// Ottiene spazio disponibile e totale (in KB) per un mount point
// es: "/", "/home", ecc.
int get_disk_usage(const char *mount_point, long *total_kb, long *available_kb);

#ifdef __cplusplus
}
#endif

#endif // SYSTEM_INFO_H
