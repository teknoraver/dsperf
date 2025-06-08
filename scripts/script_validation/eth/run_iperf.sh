#!/bin/bash

#Convert k, M, G, T suffixes
convert_to_bytes() {
  local input="$1"
  local number unit multiplier

  number=$(echo "$input" | grep -oP '^[0-9]+(\.[0-9]+)?')
  unit=$(echo "$input" | grep -oP '[kMGT]?' | tr '[:upper:]' '[:lower:]')

  case "$unit" in
    k) multiplier=1024 ;;
    m) multiplier=$((1024**2)) ;;
    g) multiplier=$((1024**3)) ;;
    t) multiplier=$((1024**4)) ;;
    "") multiplier=1 ;;  # No unit means plain bytes
    *) echo "Unit not supported: $unit" >&2; return 1 ;;
  esac

  # Use bc to handle floating-point numbers.
  bytes=$(echo "$number * $multiplier" | bc)
  # Round down.
  bytes=${bytes%.*}
  echo "$bytes"
}

#######################################################################################

# Check paramters
if [ $# -ne 4 ]; then
  echo "Use: $0 <IP_destination> <total_bytes> <repetitions> <filename_csv>"
  echo "Example: $0 192.168.1.1 1M folder data.csv"
  exit 1
fi

IP_DEST=$1
BYTES_TOTALI=$(convert_to_bytes $2)
REPETITIONS=$3
FILE_NAME="$4_iperf.csv"
#########################################################
REGEX1='s/^ *([0-9.]+) MB *\/ *([0-9.]+) sec *= *([0-9.]+) Mbps.*/\2 \1 \3/'

if [ ! -f "$FILE_NAME" ]; then
  echo "Running a test with iperf"
  echo "t.time[s] throughout[MBps]" | tr ' ' '\t' > "$FILE_NAME"
fi

# Running a test with iperf
for i in $(seq 1 $REPETITIONS); do
  iperf -c "$IP_DEST" -p 5001 -n "$BYTES_TOTALI" -f M | tail -n 1 | awk '{split($3,a,"-"); durata=a[2]; banda=$7; print durata, banda}'| tr ' ' '\t' | sed 's/\./,/g' >> "$FILE_NAME"
done

