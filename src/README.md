# dperf - Network Performance Tool

**Versione:** 0.1.0  
**Descrizione:**  
`dperf` è uno strumento per la misurazione delle prestazioni di rete (RTT e throughput) su collegamenti **underlay** o tramite rete **DaaS**. Supporta modalità *client* e *server*, e può esportare le misurazioni in un file CSV.

## 🚀 Utilizzo

### Client (Nodo A)
```bash
./dperf -h <host:port> --blocksize <bytes>|--packet-size <bytes> [-n <repetitions>|-c<ping-count>]  --underlay|--daas [-f <file.csv>] 

```

### Server (Nodo B)
```bash
./dperf -s <port> --blocksize|--packetsize --underlay|--daas
```

## ⚙️ Opzioni disponibili

| Flag/Opzione             | Descrizione                                                                                          |
|--------------------------|----------------------------------------------------------------------------------------------------|
| `-s [port]`              | Avvia il programma in modalità server (porta di default: 8080)                                     |
| `-h <host:port>`         | Indirizzo e porta del server a cui connettersi (modalità client)                                   |
| `--underlay`             | Usa la modalità rete underlay (socket IPv4)                                                        |
| `--daas`                 | Usa la modalità overlay DaaS                                                                        |
| `-f <csv_file>`          | File CSV per esportazione dei risultati (opzionale, solo client)                                  |
| `-v`                     | Mostra informazioni sulla versione ed esce                                                         |
| `--blocksize <bytes>`    | Totale dati da trasferire in byte (solo client) [range: MIN_BLOCK_SIZE - MAX_BLOCK_SIZE], esclusivo con `--packet-size` |
| `-n <repetitions>`       | Numero di ripetizioni per la trasmissione del blocco dati (solo con `--blocksize` in modalità client)                 |
| `--packet-size <bytes>`  | Dimensione di ogni pacchetto in byte [range: MIN_PACKET_SIZE - MAX_PACKET_SIZE], esclusivo con `--blocksize` |
| `-c <count>`             | Numero di pacchetti da inviare (solo con `--packet-size` in modalità client)                                          |

### NOTA
Client e server devono comunque specificare la modalità di run (--blocksize o --packetsize) senza però dover specificare la dimensione in byte di entrambi


## CSV Header file

Il file CSV contiene i risultati delle misurazioni con le seguenti colonne:

| Colonna           | Descrizione                                                                                         |
|-------------------|---------------------------------------------------------------------------------------------------|
| **Host**          | Specifiche della macchina che esegue il programma (OS, architettura, ecc.)                         |
| **Date-Time**     | Data e ora di avvio del test                                                                       |
| **Data Block Size**| Dimensione del blocco dati trasferito in byte, generato come sequenza di caratteri 'a'             |
| **Layer**         | Tipo di rete usata: Underlay o DaaS-Overlay                                                       |
| **Layer Info**    | Informazioni sul protocollo (es. versione IPv4) o versione del driver DaaS (es. INET4)             |
| **Layer Version** | Versione restituita da `get_version` dell'API DaaS o versione della libreria socket/winsocket usata |
| **Average RTT**   | Tempo medio di Round-Trip Time (RTT) misurato in millisecondi (ms)                                 |
| **Average Throughput** | Velocità media di trasferimento dati misurata in Megabit al secondo (Mbps)                     |
| **Packet Length** | Dimensione di ogni singolo pacchetto in byte                                                      |
| **Packets Sent**  | Numero totale di pacchetti inviati                                                                |
| **Packets Received** | Numero totale di pacchetti ricevuti correttamente                                                |
| **Packets Lost**  | Numero di pacchetti persi durante la trasmissione                                                 |
| **Repetitions**   | Numero di ripetizioni effettuate (se applicabile, es. numero di ping nel client)                   |
| **Total Bytes Sent** | Totale byte inviati nel test (somma di tutti i pacchetti o blocchi inviati)                      |

### NOTA
Il tipo di dati scritti sul file csv dipenderà dalla modalità (--blocksize o --packet-size) scelta.
