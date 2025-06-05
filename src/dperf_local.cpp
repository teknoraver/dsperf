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

    // Inizializzazione
    for (int i = 0; i < MAX_LINKS; ++i) {
        setup->links.links[i] = _LINK_NONE;
        setup->links.uris[i] = "";
    }
    for (int i = 0; i < MAX_REMOTE_LINKS; ++i) {
        setup->maps.remote_links[i] = _LINK_NONE;
        setup->maps.uris[i] = "";
    }

    char line[MAX_LINE_LEN];
    bool in_map_section = false;
    int link_index = 0;
    int map_index = 0;

    while (fgets(line, sizeof(line), file)) {
        // Rimuovi newline e spazi
        char *pos;
        if ((pos = strchr(line, '\n'))) *pos = '\0';

        if (strlen(line) == 0 || isspace(line[0])) continue;

        if (strcmp(line, "MAP") == 0) {
            in_map_section = true;
            continue;
        }

        if (!in_map_section) {
            if (setup->local_din == 0) {
                setup->local_din = atoi(line);
            } else {
                int link;
                char uri[256];
                if (sscanf(line, "%d %255s", &link, uri) == 2 && link_index < MAX_LINKS) {
                    setup->links.links[link_index] = parse_link_type(link);
                    setup->links.uris[link_index] = strdup(uri); 
                    link_index++;
                }
            }
        } else {
            int din, link;
            char uri[256];
            if (sscanf(line, "%d %d %255s", &din, &link, uri) == 3 && map_index < MAX_REMOTE_LINKS) {
                setup->maps.remote_dins[map_index] = din;
                setup->maps.remote_links[map_index] = parse_link_type(link);
                setup->maps.uris[map_index] = strdup(uri);
                map_index++;
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
    args->run_mode = -1;
    args->block_size = 0;
    args->packet_size = 0;
    args->repetitions = 1;
    args->csv_enabled = false;
    args->pings = 1;

    static struct option long_options[] = {
        {"underlay", no_argument, 0, 1},
        {"daas", required_argument, 0, 2},
        {"blocksize", optional_argument, 0, 3},
        {"packet-size", required_argument, 0, 4},
        {0, 0, 0, 0}
    };

    int option_index = 0;
    int c;
    while ((c = getopt_long(argc, argv, "S:s:n:c:f:v", long_options, &option_index))) {
        if (c == -1) break;

        switch (c) {
            case 'S':
                if (args->is_sender != -1) {
                    fprintf(stderr, "Error: Cannot specify both -S and -s\n");
                    exit(EXIT_FAILURE);
                }
                args->is_sender = 0;
                args->port = atoi(optarg);
                break;
            case 's':
                if (args->is_sender != -1) {
                    fprintf(stderr, "Error: Cannot specify both -S and -s\n");
                    exit(EXIT_FAILURE);
                }
                args->is_sender = 1;
                strncpy(args->remote_ip, optarg, sizeof(args->remote_ip) - 1);
                break;
            case 'n':
                args->repetitions = atoi(optarg);
                break;
            case 'c':
                args->pings = atoi(optarg);
                break;
            case 'f':
                args->csv_enabled = 1;
                strncpy(args->csv_path, optarg, sizeof(args->csv_path) - 1);
                break;
            case 'v':
                args->version = 1;
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
                if (args->run_mode != -1) {
                    fprintf(stderr, "Error: Cannot specify both --blocksize and --packet-size\n");
                    exit(EXIT_FAILURE);
                }
                args->run_mode = 0;
                if (optarg) {
                    args->block_size = atoi(optarg);
                } else {
                    args->block_size = 4096; // default value
                }
                break;
            case 4: // --packet-size
                if (args->run_mode != -1) {
                    fprintf(stderr, "Error: Cannot specify both --blocksize and --packet-size\n");
                    exit(EXIT_FAILURE);
                }
                args->run_mode = 1;
                args->packet_size = atoi(optarg);
                break;
            default:
                print_usage(argv[0]);
                exit(EXIT_FAILURE);
        }
    }
}

int validate_args(program_args_t *args, const char *prog_name) {
    // Check required modes
    if (args->is_sender == -1) {
        fprintf(stderr, "Error: Must specify either -S (server) or -s (client)\n");
        return EXIT_FAILURE;
    }

    if (args->layer_mode == -1) {
        fprintf(stderr, "Error: Must specify either --underlay or --daas\n");
        return EXIT_FAILURE;
    }

    if (args->run_mode == -1) {
        fprintf(stderr, "Error: Must specify either --blocksize or --packet-size\n");
        return EXIT_FAILURE;
    }

    // Client-specific validations
    if (args->is_sender == 1) {
        if (args->run_mode == 1 && args->packet_size <= 0) {
            fprintf(stderr, "Error: Client must specify positive packet size with --packet-size\n");
            return EXIT_FAILURE;
        }
    }

    // Server-specific defaults
    if (args->is_sender == 0) {
        args->block_size = 0;
    }

    return EXIT_SUCCESS;
}