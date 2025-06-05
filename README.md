# dperf - DaaS Network Performance Tool

**Versione:** 0.0.33

## Authors:
- sebastiano.meduri@gmail.com
- l.grillo@sebyone.it
- m.pagano@sebyone.it


## Overview:

**dperf** is a network benchmarking tool designed to evaluate **performance** of standard networks by different methods: trip-time, traffic generator and much more. In addition, dperf allow to compare the performance of **underlay** and **overlay** network layers.
many of the tools available, while effective for standard IP networks, do not fully capture performance characteristics in modern, heterogeneous overlay network contexts, because they bypass the application layer and rely on specific IP protocols and subprotocols.

dperf uses a loopback-based methodology between two nodes (source and destination) to assess throughput, by leveraging native IPv4 sockets and the `libdaas` API with the a choosen driver.

## Why dperf?

- Independency on IP-specific protocols (e.g., ICMP).
- Process full-stack traversal.
- Full compatibility with abstracted and virtualized networking models.
- Measures bandwidth at both physical and logical layers
- Compatible with heterogeneous and virtualized environments
- Built with socket-based communication and libdaas support
- Enables side-by-side evaluation of underlay and overlay performance

## Key Concepts

The performances of a network can be evaluated with respect to four factors: **capabilities**, **availability**, **bandwidth** and **security**.

- **Capabilities**: Summarizes the functional capabilities.
- **Availability**: Accessibility and uptime of the network.
- **Bandwidth**: Refers to the rate of data that may be delivered over physical or logical links.
- **Security**: Summarizes the ability of the network to ensure the peers identification, confidentiality and integrity of the transferred information.


An **overlay network** is a model for defining one or more levels of abstraction of the physical network through software. The aim is to run multiple separate virtualized networks on the same physical infrastructure.

In an overlay network context, the **underline** and **overline** levels are identified. The technical resources, devices and protocols used to transport data are considered to belong to the underline layer.  Overlay layer refers to the software technologies and logical models adopted to virtualize access to the resources of the underline layer.

The use of overlay technologies brings numerous advantages and some relevant disadvantages.
Overlay brings incontrovertible benefits in terms of resilience, versatility and adaptability, however they require additional maintenance practices in addition to consuming network bandwidth, therefore a reduction in the network ability to transfer data.




---

**dperf** fills the gap where traditional tools fall short, offering precision and adaptability for next-generation network performance evaluation.




## ⚙️ Available flags


| Flag               | Description                                                                                          |
|--------------------------|----------------------------------------------------------------------------------------------------|
| `-v`                     | Show version info                                                         |
| `-V`                     | Show extended version info                                                       |
| `--h`                    | Show help                                                                              |
| `-S [port]`              | Starts server (default port 5001)                                     |
| `-s <host:port>`         | Connect to remote host (client mode)                                   |
| `--protocol`             | !! Select the desired protocol for the test (UDP, TCP/IP, ecc.)  default TCP/IP                 |
| `--driver`               | !! Specifics the driver to use  |
| `--overlay`              | !! Runs the test in overlay mode (daas)     |
| `--underlay`             | Runs the test in underlay mode             |
| `--blocksize <bytes>`    | Total data to transfer (in bytes), only client have to specify the size |
| `-n <repetitions>`       | Specify how many times the test have to run               |
| `-f <csv_file>`          | Export the data in a CSV file                               |
| `-y`                     | Formats output console in csv                                                                     |


## CSV Header file

CSV File contains those informations:

| **Field**                     | **Description**                                                                                 |
|------------------------------|--------------------------------------------------------------------------------------------------|
| **Host**                     | Specifications of the machine running the program (e.g. OS, architecture, etc.)                |
| **Date and time**               | Test start date and time                                                                    |
| **Data Block**   | Size of the transferred data block (in bytes), generated as a character sequence `'a'` |
| **Layer**                    | Type of network used: Underlay or DaaS-Overlay                                                |
| **Layer info**       | Protocol details (e.g. IPv4) or DaaS driver (e.g. INET4)                                    |
| **Layer Versionr**           | Version of the DaaS API (`get_version`) or socket library used                      |

## Measurable data

| **Field**                     | **Description**                                                                                 |
|------------------------------|--------------------------------------------------------------------------------------------------|
| **Transfer Time**   | Time taken to transfer the data block (in milliseconds)                                |
| **Total Data sent**      | Total size of the block sent from the client to the server (in bytes)                             |
| **Throughput**               | Average transfer speed expressed in MBps and Mbps                                         |
| **RTT**                      | Round-Trip Time calculated before sending the block, in milliseconds (ms)                   |




## Usage

### Client (Node A)
```bash
./dperf -s <host:port> --blocksize <bytes>|--packet-size <bytes> [-n <repetitions>|-c<ping-count>]  --underlay|--daas [-f <file.csv>] 

```
### Server (Node B)
```bash
./dperf -S <port> --blocksize|--packetsize --underlay|--daas
```

## Compile

Run the following commands in root directory

```bash
cmake .
cmake --build .
```
You can choose to not link DaaS with
```bash
cmake -DNODAAS=ON .
```
Then you can run the executable `dperf`!

## Examples

Transfer of a data block of 10 MB

### start server
./dperf -S --underlay --blocksize

### start client
./dperf --underlay -s 127.0.0.1:5001 --blocksize 1024000000 -y -n 10 > data_100M.csv

sed 's/\\./,/g' data_100M.csv > datav_100M.csv

or:

./dperf --underlay -s 127.0.0.1:5001 --blocksize 1024000000 -y -n 10 | sed 's/\\./,/g' > datav_100M.csv

### start client (overlay mode)

./dperf -s 127.0.0.1:2001 --daas node_setup.ini --blocksize 1024000000
