#include "system_info.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#if defined(__linux__) || defined(__RASP__)
#include <sys/statvfs.h>
#include <sys/utsname.h>


#define CPU_TEMP_PATH "/sys/class/thermal/thermal_zone0/temp"

// ------------------------
// OS INFO
// ------------------------

const char* get_os_name() {
    static char os_name[128];
    struct utsname buffer;
    if (uname(&buffer) != 0) {
        return "Unknown OS";
    }
    snprintf(os_name, sizeof(os_name), "%s %s", buffer.sysname, buffer.release);
    return os_name;
}

const char* get_architecture() {
    static char arch[128];
    struct utsname buffer;
    if (uname(&buffer) != 0) {
        return "Unknown Architecture";
    }
    snprintf(arch, sizeof(arch), "%s %s", buffer.machine, buffer.version);
    return arch;
}

const char* get_kernel_version() {
    static char version[128];
    struct utsname buffer;
    if (uname(&buffer) == 0) {
        snprintf(version, sizeof(version), "%s", buffer.release);
    } else {
        strcpy(version, "Unknown kernel version");
    }
    return version;
}
// ------------------------
// CPU MODEL
// ------------------------
void get_cpu_model(char *buffer, unsigned int size) {
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (!fp) {
        if (size > 0) buffer[0] = '\0';
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "model name", 10) == 0) {
            char *colon = strchr(line, ':');
            if (colon) {
                colon += 2;
                strncpy(buffer, colon, size - 1);
                buffer[size - 1] = '\0';
                char *newline = strchr(buffer, '\n');
                if (newline) *newline = '\0';
                fclose(fp);
                return;
            }
        }
    }
    // fallback se non trovato
    buffer[0] = '\0';
    fclose(fp);
}

// ------------------------
// CPU CORES
// ------------------------
int get_cpu_cores() {
    FILE *fp = fopen("/proc/cpuinfo", "r");
    if (!fp) return -1;

    int cores = 0;
    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        if (strncmp(line, "processor", 9) == 0) {
            cores++;
        }
    }
    fclose(fp);
    return cores;
}

// ------------------------
// MEMINFO KB
// ------------------------
long get_meminfo_kb(const char *label) {
    FILE *fp = fopen("/proc/meminfo", "r");
    if (!fp) return -1;

    char key[64];
    long value = -1;
    char unit[16];

    while (fscanf(fp, "%63s %ld %15s", key, &value, unit) == 3) {
        if (strcmp(key, label) == 0) {
            fclose(fp);
            return value;
        }
    }
    fclose(fp);
    return -1;
}

// ------------------------
// MEMORY USAGE
// ------------------------
int get_memory_usage(long *total, long *free, long *used) {
    long mem_total = get_meminfo_kb("MemTotal:");
    long mem_free = get_meminfo_kb("MemFree:");
    long buffers = get_meminfo_kb("Buffers:");
    long cached = get_meminfo_kb("Cached:");

    if (mem_total < 0 || mem_free < 0) return -1;

    *total = mem_total;
    *free = mem_free + buffers + cached;
    *used = mem_total - *free;
    return 0;
}

// ------------------------
// LOAD AVERAGE
// ------------------------
int get_load_average(float *avg1, float *avg5, float *avg15) {
    FILE *fp = fopen("/proc/loadavg", "r");
    if (!fp) return -1;

    if (fscanf(fp, "%f %f %f", avg1, avg5, avg15) != 3) {
        fclose(fp);
        return -1;
    }
    fclose(fp);
    return 0;
}

// ------------------------
// NETWORK USAGE PER INTERFACCIA
// ------------------------
int get_net_usage(const char *iface, long *rx_bytes, long *tx_bytes) {
    FILE *fp = fopen("/proc/net/dev", "r");
    if (!fp) return -1;

    char line[512];
    // Skip headers
    fgets(line, sizeof(line), fp);
    fgets(line, sizeof(line), fp);

    int found = 0;
    while (fgets(line, sizeof(line), fp)) {
        char *colon = strchr(line, ':');
        if (!colon) continue;

        *colon = '\0';
        char *name = line;
        while (*name == ' ') name++;

        if (strcmp(name, iface) == 0) {
            long rx, tx;
            // Format: RX bytes is first, TX bytes after 8 columns
            sscanf(colon + 1, "%ld %*s %*s %*s %*s %*s %*s %*s %ld", &rx, &tx);
            *rx_bytes = rx;
            *tx_bytes = tx;
            found = 1;
            break;
        }
    }

    fclose(fp);
    return found ? 0 : -1;
}

// ------------------------
// NETWORK USAGE PER INTERFACCIA (DETAILED)
// ------------------------
int get_net_usage_detailed(const char *iface, net_stats_t *stats) {
    FILE *fp = fopen("/proc/net/dev", "r");
    if (!fp) return -1;

    char line[512];
    // Skip header 2 righe
    fgets(line, sizeof(line), fp);
    fgets(line, sizeof(line), fp);

    int found = 0;
    while (fgets(line, sizeof(line), fp)) {
        char *colon = strchr(line, ':');
        if (!colon) continue;

        *colon = '\0';
        char *name = line;
        while (*name == ' ') name++;

        if (strcmp(name, iface) == 0) {
            // Parsing: 16 valori dopo il colon
            // RX: bytes packets errs drop fifo frame compressed multicast
            // TX: bytes packets errs drop fifo colls carrier compressed
            long vals[16];
            if (sscanf(colon + 1,
                       "%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld",
                       &vals[0], &vals[1], &vals[2], &vals[3], &vals[4], &vals[5], &vals[6], &vals[7],
                       &vals[8], &vals[9], &vals[10], &vals[11], &vals[12], &vals[13], &vals[14], &vals[15]) != 16) {
                fclose(fp);
                return -1;
            }
            stats->rx_bytes      = vals[0];
            stats->rx_packets    = vals[1];
            stats->rx_errs       = vals[2];
            stats->rx_drop       = vals[3];
            stats->rx_fifo       = vals[4];
            stats->rx_frame      = vals[5];
            stats->rx_compressed = vals[6];
            stats->rx_multicast  = vals[7];
            stats->tx_bytes      = vals[8];
            stats->tx_packets    = vals[9];
            stats->tx_errs       = vals[10];
            stats->tx_drop       = vals[11];
            stats->tx_fifo       = vals[12];
            stats->tx_colls      = vals[13];
            stats->tx_carrier    = vals[14];
            stats->tx_compressed = vals[15];

            found = 1;
            break;
        }
    }

    fclose(fp);
    return found ? 0 : -1;
}

// ------------------------
// ELENCO INTERFACCE DI RETE
// ------------------------
int get_network_interfaces(char interfaces[][MAX_IFACE_NAME]) {
    FILE *fp = fopen("/proc/net/dev", "r");
    if (!fp) return -1;

    char line[512];
    int count = 0;

    fgets(line, sizeof(line), fp);
    fgets(line, sizeof(line), fp);

    while (fgets(line, sizeof(line), fp) && count < MAX_INTERFACES) {
        char *colon = strchr(line, ':');
        if (!colon) continue;

        *colon = '\0';
        char *name = line;
        while (*name == ' ') name++;

        strncpy(interfaces[count], name, MAX_IFACE_NAME - 1);
        interfaces[count][MAX_IFACE_NAME - 1] = '\0';
        count++;
    }
    fclose(fp);
    return count;
}

// ------------------------
// USO TOTALE RETE SU TUTTE LE INTERFACCE
// ------------------------
int get_total_net_usage(long *total_rx, long *total_tx) {
    char interfaces[MAX_INTERFACES][MAX_IFACE_NAME];
    int count = get_network_interfaces(interfaces);
    if (count < 0) return -1;

    long rx_sum = 0, tx_sum = 0;
    for (int i = 0; i < count; ++i) {
        net_stats_t stats;
        if (get_net_usage_detailed(interfaces[i], &stats) == 0) {
            rx_sum += stats.rx_bytes;
            tx_sum += stats.tx_bytes;
        }
    }
    *total_rx = rx_sum;
    *total_tx = tx_sum;
    return 0;
}

// ------------------------
// TEMPERATURA CPU
// ------------------------
float get_cpu_temperature() {
    FILE *fp = fopen(CPU_TEMP_PATH, "r");
    if (!fp) return -1.0f;

    int millideg = 0;
    if (fscanf(fp, "%d", &millideg) != 1) {
        fclose(fp);
        return -1.0f;
    }
    fclose(fp);

    return millideg / 1000.0f;
}

// ------------------------
// UTILIZZO DISCO
// ------------------------
int get_disk_usage(const char *mount_point, long *total_kb, long *available_kb) {
    struct statvfs stat;
    if (statvfs(mount_point, &stat) != 0) {
        return -1;
    }
    *total_kb = (stat.f_blocks * stat.f_frsize) / 1024;
    *available_kb = (stat.f_bavail * stat.f_frsize) / 1024;
    return 0;
}
#endif // end of __linux__
