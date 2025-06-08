#!/usr/bin/env bash

# File temporaneo per salvare i PID
PID_FILE="/tmp/benchmark_servers.pid"

start_servers() {
    echo -e "Starting servers for benchmark analysis"

    echo -e "Starting NUTTCP server on port 9001"
    nuttcp -S -p 9001 &> /dev/null &
    echo $! >> "$PID_FILE"

    echo -e "Starting IPERF server on port 5001"
    iperf -s &> /dev/null &
    echo $! >> "$PID_FILE"

    echo -e "Starting NETPERF server on port 9002"
    netserver -p 9002 &> /dev/null &
    echo $! >> "$PID_FILE"
    
    echo -e "Starting DPERF server on port 5002"
    dperf -S 5002 --underlay &> /dev/null &
    echo $! >> "$PID_FILE"

    echo -e "✅ All servers started. PIDs saved to $PID_FILE"
}

stop_servers() {
    if [[ -f "$PID_FILE" ]]; then
        echo -e "Stopping benchmark servers"
        while read -r pid; do
            if kill -0 "$pid" 2>/dev/null; then
                kill "$pid" && echo "🔴 Stopped process $pid"
            fi
        done < "$PID_FILE"
        rm -f "$PID_FILE"
    else
        echo "❌ No PID file found. Are the servers running?"
    fi
}

case "$1" in
    --start)
        start_servers
        ;;
    --stop)
        stop_servers
        ;;
    *)
        echo "Usage: $0 --start | --stop"
        ;;
esac
