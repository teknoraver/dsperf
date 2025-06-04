# System Info

## Descrizione
`system_info` è una libreria C leggera per sistemi Linux che permette di ottenere informazioni dettagliate sulla macchina su cui gira il software. Senza dipendenze esterne complesse, utilizza principalmente l’accesso diretto ai file di sistema come `/proc` e `/sys` per raccogliere dati su CPU, memoria, rete e disco.

## Funzionalità

- **CPU**
  - Ottieni il modello della CPU (es. "Intel(R) Core(TM) i7-8565U CPU @ 1.80GHz")
  - Numero di core logici
  - Temperatura attuale della CPU (se disponibile)

- **Memoria**
  - Memoria totale, libera e usata (in KB)
  - Lettura diretta da `/proc/meminfo`

- **Carico di sistema**
  - Load average a 1, 5 e 15 minuti (carico medio CPU)

- **Rete**
  - Elenco delle interfacce di rete attive
  - Statistiche dettagliate per ogni interfaccia, le informazioni dettagliate possono essere visualizzate nelle sezione successiva
  - Statistiche di rete aggregate su tutte le interfacce

- **Disco**
  - Spazio totale e disponibile per un mount point specificato (es. `/`)

  ---

## Funzioni Principali

```c
void get_cpu_model(char *buffer, unsigned int size);
int get_cpu_cores();
float get_cpu_temperature();

long get_meminfo_kb(const char *label);
int get_memory_usage(long *total, long *free, long *used);
int get_load_average(float *avg1, float *avg5, float *avg15);

int get_network_interfaces(char interfaces[MAX_INTERFACES][MAX_IFACE_NAME]);
int get_net_usage(const char *iface, long *rx_bytes, long *tx_bytes);
int get_net_usage_detailed(const char *iface, net_stats_t *stats);
int get_total_net_usage(long *total_rx, long *total_tx);

int get_disk_usage(const char *mount_point, long *total_kb, long *available_kb);

```

---

## Struttura rete in dettaglio `net_stats_t`

```c
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
```

## Dettaglio dei campi rete

Questi dati sono ricavati da `/proc/net/dev`, riga relativa a ciascuna interfaccia:

| Campo           | Significato                                                                                 |
|-----------------|---------------------------------------------------------------------------------------------|
| **rx_bytes**      | Numero totale di byte ricevuti (ricezione)                                                 |
| **rx_packets**    | Numero totale di pacchetti ricevuti                                                        |
| **rx_errs**       | Numero di errori nella ricezione                                                           |
| **rx_drop**       | Numero di pacchetti scartati in ricezione                                                  |
| **rx_fifo**       | Numero di errori FIFO in ricezione                                                         |
| **rx_frame**      | Numero di errori di frame in ricezione (es. errori di allineamento)                        |
| **rx_compressed** | Numero di pacchetti ricevuti compressi                                                     |
| **rx_multicast**  | Numero di pacchetti multicast ricevuti                                                     |
| **tx_bytes**      | Numero totale di byte trasmessi (trasmissione)                                            |
| **tx_packets**    | Numero totale di pacchetti trasmessi                                                      |
| **tx_errs**       | Numero di errori in trasmissione                                                           |
| **tx_drop**       | Numero di pacchetti scartati in trasmissione                                              |
| **tx_fifo**       | Numero di errori FIFO in trasmissione                                                      |
| **tx_colls**      | Numero di collisioni nella trasmissione                                                    |
| **tx_carrier**    | Numero di errori di carrier nella trasmissione                                            |
| **tx_compressed** | Numero di pacchetti trasmessi compressi             