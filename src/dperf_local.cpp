#include "dperf_local.hpp"

#ifdef WITH_DAAS
link_t parse_link_type(int val) {
    if (val >= _LINK_NONE && val < LINK_MAX_VAL){
        return (link_t)val;
    }
    else {
        return _LINK_NONE; 
    }     
}
#endif

void print_usage(const char *prog_name)
{
    printf("Usage:\n");
    printf("Note: the <bytes> part of --blocksize and --packet-size of the following commands are only\n");
    printf("considered in client mode.\n");
    printf("\n");
    printf("  Sender (Client): %s -s <host:port> [--blocksize <bytes> -n <repetitions> | --packet-size <bytes> -c <count>] --underlay|--daas [-f <csv_file>]\n", prog_name);
    printf("  Receiver (Server): %s -S [port] --underlay|--daas --blocksize|--packet-size\n", prog_name);
}

void print_options(const char *prog_name)
{
    printf("\nOptions:\n");
    printf("  -S [port]              Run in server mode (default port: 8080)\n");
    printf("  -s <host:port>         Server address and port to connect to (client mode)\n");
    printf("  --underlay             Use underlay network mode (IPv4 sockets)\n");
    printf("  --daas                 Use DaaS overlay mode\n");
    printf("  -f <csv_file>          CSV file for results output (optional, client only)\n");
    printf("  -v                     Show version information and exit\n");
    printf("\n");
    printf("Note: the <bytes> part of the following commands are considered only in client mode\n");
    printf("  --blocksize <bytes>    Total data to transfer in bytes [%d-%d] (mutually exclusive with --packet-size)\n", MIN_BLOCK_SIZE, MAX_BLOCK_SIZE);
    printf("  -n <repetitions>       Number of repetitions for data block transmission (only with --blocksize)\n");
    printf("  --packet-size <bytes>  Individual packet size in bytes [%d-%d] (mutually exclusive with --blocksize)\n", MIN_PACKET_SIZE, MAX_PACKET_SIZE);
    printf("  -c <count>             Number of packets to send (only with --packet-size)\n");
    printf("\n");
}

#ifdef WITH_DAAS
bool parse_daas_ini(const char* filepath, daas_setup_t *setup) {
    FILE* file = fopen(filepath, "r");
    if (!file) {
        perror("Failed to open overlay file");
        return false;
    }

    // Inizializza valori
    setup->local_din = 0;
    for (int i = 0; i < MAX_LINKS; i++) {
        setup->links.links[i] = _LINK_NONE;
        setup->links.uris[i] = NULL;
    }
    for (int i = 0; i < MAX_REMOTE_LINKS; i++) {
        setup->maps.remote_dins[i] = 0;
        setup->maps.remote_links[i] = _LINK_NONE;
        setup->maps.uris[i] = NULL;
    }

    char line[MAX_LINE_LEN];
    enum { NONE=0, LOCAL, MAP } current_section = NONE;

    while (fgets(line, sizeof(line), file)) {
        // Rimuovi newline
        char *pos;
        if ((pos = strchr(line, '\n'))) *pos = '\0';

        // Trim spazi iniziali
        char* start = line;
        while (isspace(*start)) start++;

        // Ignora righe vuote e commenti
        if (*start == '\0' || *start == '#' || *start == ';') continue;

        // Sezione
        if (*start == '[') {
            if (strncmp(start, "[LOCAL]", 7) == 0) {
                current_section = LOCAL;
            } else if (strncmp(start, "[MAP]", 5) == 0) {
                current_section = MAP;
            } else {
                current_section = NONE; // sezione ignota
            }
            continue;
        }

        // Parsing key=value
        char key[128], value[256];
        if (sscanf(start, "%127[^=]=%255s", key, value) != 2) {
            // riga non conforme, salta
            continue;
        }

        // Ora processa in base alla sezione
        if (current_section == LOCAL) {
            if (strcmp(key, "DIN") == 0) {
                setup->local_din = atoi(value);
            } else if (strncmp(key, "LINK_", 5) == 0) {
                int idx = atoi(key + 5);
                if (idx >= 0 && idx < MAX_LINKS) {
                    setup->links.links[idx] = parse_link_type(atoi(value));
                }
            } else if (strncmp(key, "URI_", 4) == 0) {
                int idx = atoi(key + 4);
                if (idx >= 0 && idx < MAX_LINKS) {
                    free(setup->links.uris[idx]); // libera vecchio se presente
                    setup->links.uris[idx] = strdup(value);
                }
            }
        } else if (current_section == MAP) {
            if (strncmp(key, "REMOTE_DIN_", 11) == 0) {
                int idx = atoi(key + 11);
                if (idx >= 0 && idx < MAX_REMOTE_LINKS) {
                    setup->maps.remote_dins[idx] = atoi(value);
                }
            } else if (strncmp(key, "REMOTE_LINK_", 12) == 0) {
                int idx = atoi(key + 12);
                if (idx >= 0 && idx < MAX_REMOTE_LINKS) {
                    setup->maps.remote_links[idx] = parse_link_type(atoi(value));
                }
            } else if (strncmp(key, "REMOTE_URI_", 11) == 0) {
                int idx = atoi(key + 11);
                if (idx >= 0 && idx < MAX_REMOTE_LINKS) {
                    free(setup->maps.uris[idx]);
                    setup->maps.uris[idx] = strdup(value);
                }
            }
        }
    }

    fclose(file);
    return true;
}

#endif

void parse_args(int argc, char *argv[], program_args_t *args) {
    memset(args, 0, sizeof(*args));
    args->is_sender = -1;
    args->layer_mode = -1;
    args->block_size = 0;
    args->mtu_defined = false;
    args->mtu_size = 1500;
    args->repetitions = 1;
    args->pack_num = 1;
    args->csv_enabled = false;
    args->csv_no_header = false;
    args->version = false;
    args->port = 0;
    args->remote_din = -1;
    args->remote_ip[0] = '\0';
    args->overlay_path[0] = '\0';
    args->csv_path[0] = '\0';
    args->use_udp = false;

    static struct option long_options[] = {
        {"underlay", no_argument, 0, 1},
        {"daas", required_argument, 0, 2},
        {"blocksize", required_argument, 0, 3},
        {"udp", no_argument, 0, 4},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    int c;

    while ((c = getopt_long(argc, argv, "S:s:n:c:m:f:t:y:v", long_options, &option_index)) != -1) {
        switch (c) {
            case 'S':
                if (args->is_sender != -1) {
                    fprintf(stderr, "Error: Cannot specify both -S and -s\n");
                    exit(EXIT_FAILURE);
                }
                args->is_sender = 0;
                if (args->layer_mode == 0) { // underlay: PORT
                    args->port = atoi(optarg);
                    if (args->port <= 0) {
                        fprintf(stderr, "Error: Invalid port number for server\n");
                        exit(EXIT_FAILURE);
                    }
                } else if (args->layer_mode == 1) {
                    args->remote_din = atoi(optarg);
                    if (args->remote_din < 0) {
                        fprintf(stderr, "Error: Invalid DIN for server\n");
                        exit(EXIT_FAILURE);
                    }
                } else {
                    // Layer mode non ancora definito: salvare comunque arg e fare check dopo
                    // Conserviamo il valore provvisorio in remote_ip o remote_din, controlliamo in validate
                    // Per semplicità: useremo remote_ip come buffer temporaneo per il valore
                    strncpy(args->remote_ip, optarg, sizeof(args->remote_ip) - 1);
                }
                break;

            case 's':
                if (args->is_sender != -1) {
                    fprintf(stderr, "Error: Cannot specify both -S and -s\n");
                    exit(EXIT_FAILURE);
                }
                args->is_sender = 1;
                if (args->layer_mode == 0) { // underlay: IP:PORT
                    strncpy(args->remote_ip, optarg, sizeof(args->remote_ip) - 1);
                } else if (args->layer_mode == 1) {
                    args->remote_din = atoi(optarg);
                    if (args->remote_din < 0) {
                        fprintf(stderr, "Error: Invalid DIN for client\n");
                        exit(EXIT_FAILURE);
                    }
                } else {
                    // Layer mode non ancora definito: salvare in remote_ip e validare dopo
                    strncpy(args->remote_ip, optarg, sizeof(args->remote_ip) - 1);
                }
                break;

            case 'n':
                args->repetitions = atoi(optarg);
                if (args->repetitions < 1) {
                    fprintf(stderr, "Error: repetitions must be >= 1\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'c':
                args->pack_num = atoi(optarg);
                if(args->pack_num < 1) {
                    fprintf(stderr, "Error: packet number must be >= 1\n");
                    exit(EXIT_FAILURE);
                }
                break;

            case 'm':
                args->mtu_defined = true;
                args->mtu_size = atoi(optarg);
                if (args->mtu_size < 1) {
                    fprintf(stderr, "Error: MTU must be >= 1\n");
                    exit(EXIT_FAILURE);
                }
                break;

            case 'f':
            {
                args->csv_enabled = true;
                strncpy(args->csv_path, optarg, sizeof(args->csv_path) - 1);
                break;
            }
            
            case 't':
            {
                args->time_defined = true;
                int val = atoi(optarg);
                args->time = val;
                break;
            }
            case 'y':
            {
                args->csv_format = true;
                args->csv_no_header = false;

                int val = atoi(optarg);
                if (val != 0 && val != 1) {
                    fprintf(stderr, "Errore: il valore per --csv-no-header (-y) deve essere 0 o 1.\n");
                    exit(EXIT_FAILURE);
                }

                args->csv_no_header = (val == 1);
            }
            break;

            case 'v':
                args->version = true;
                break;

            case 1: // --underlay
                if (args->layer_mode != -1) {
                    fprintf(stderr, "Error: Cannot specify both --underlay and --daas\n");
                    exit(EXIT_FAILURE);
                }
                args->layer_mode = 0;
                break;

            case 2: // --daas
                if (args->layer_mode != -1) {
                    fprintf(stderr, "Error: Cannot specify both --underlay and --daas\n");
                    exit(EXIT_FAILURE);
                }
                args->layer_mode = 1;
                strncpy(args->overlay_path, optarg, sizeof(args->overlay_path) - 1);
                break;

            case 3: // --blocksize
                args->block_size = atoi(optarg);
                if (args->block_size < 1) {
                    fprintf(stderr, "Error: blocksize must be >= 1\n");
                    exit(EXIT_FAILURE);
                }
                break;

			case 4:  // --udp
                args->use_udp = true;
                break;

            default:
                fprintf(stderr, "Unknown option\n");
                exit(EXIT_FAILURE);
        }
    }
}

int validate_args(program_args_t *args, const char *prog_name) {
    // Controlli base
    if (args->is_sender == -1) {
        fprintf(stderr, "Error: Must specify either -S (server) or -s (client)\n");
        return EXIT_FAILURE;
    }
    if (args->layer_mode == -1) {
        fprintf(stderr, "Error: Must specify either --underlay or --daas\n");
        return EXIT_FAILURE;
    }

    // Per server: accetta solo parametri base
    if (args->is_sender == 0) {
        // Se layer_mode non è definito, proviamo a dedurlo dall'argomento di -S
        if (args->layer_mode == 0) {
            // -S deve essere porta
            if (args->port == 0) {
                // Se non settata ancora, proviamo a convertire da remote_ip (tmp)
                if (args->remote_ip[0] != '\0') {
                    args->port = atoi(args->remote_ip);
                    if (args->port <= 0) {
                        fprintf(stderr, "Error: Invalid port number for server\n");
                        return EXIT_FAILURE;
                    }
                } else {
                    fprintf(stderr, "Error: Server port not specified\n");
                    return EXIT_FAILURE;
                }
            }
        } else if (args->layer_mode == 1) {
            // -S deve essere remote_din
            if (args->remote_din < 0) {
                if (args->remote_ip[0] != '\0') {
                    args->remote_din = atoi(args->remote_ip);
                    if (args->remote_din < 0) {
                        fprintf(stderr, "Error: Invalid DIN for server\n");
                        return EXIT_FAILURE;
                    }
                } else {
                    fprintf(stderr, "Error: Server DIN not specified\n");
                    return EXIT_FAILURE;
                }
            }
        }

        // Verifica che non siano presenti opzioni non ammesse per server
        if (args->block_size != 0) {
            fprintf(stderr, "Error: Server must not specify --blocksize\n");
            return EXIT_FAILURE;
        }
        if (args->repetitions != 1) {
            fprintf(stderr, "Error: Server must not specify -n (repetitions)\n");
            return EXIT_FAILURE;
        }
        if (args->csv_enabled) {
            fprintf(stderr, "Error: Server must not specify -f (csv output)\n");
            return EXIT_FAILURE;
        }
        if (args->csv_no_header) {
            fprintf(stderr, "Error: Server must not specify -y (csv header control)\n");
            return EXIT_FAILURE;
        }
        if (args->mtu_defined) {
            fprintf(stderr, "Error: Server must not specify -m (mtu)\n");
            return EXIT_FAILURE;
        }
    }

    // Per client: deve avere tutti i parametri corretti
    if (args->is_sender == 1) {
        // Verifica layer_mode e argomenti collegati
        if (args->layer_mode == 0) {
            // underlay: remote_ip deve essere IP:PORT
            if (args->remote_ip[0] == '\0') {
                fprintf(stderr, "Error: Client must specify IP:PORT for underlay\n");
                return EXIT_FAILURE;
            }
        } else if (args->layer_mode == 1) {
            // daas: remote_din >= 0
            if (args->remote_din < 0) {
                if (args->remote_ip[0] != '\0') {
                    args->remote_din = atoi(args->remote_ip);
                    if (args->remote_din < 0) {
                        fprintf(stderr, "Error: Invalid DIN for client\n");
                        return EXIT_FAILURE;
                    }
                } else {
                    fprintf(stderr, "Error: Client DIN not specified\n");
                    return EXIT_FAILURE;
                }
            }
        }

        if (args->block_size < 1) {
            fprintf(stderr, "Error: Client must specify --blocksize >= 1\n");
            return EXIT_FAILURE;
        }
        if (args->repetitions < 1) {
            fprintf(stderr, "Error: repetitions must be >= 1\n");
            return EXIT_FAILURE;
        }
        if (args->mtu_defined && args->mtu_size < 1) {
            fprintf(stderr, "Error: MTU must be >= 1\n");
            return EXIT_FAILURE;
        }
        // csv_path se csv_enabled deve essere valorizzato
        if (args->csv_enabled && args->csv_path[0] == '\0') {
            fprintf(stderr, "Error: CSV output enabled but no file specified\n");
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
