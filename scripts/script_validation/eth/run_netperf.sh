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
FILE_NAME="$4_netperf.csv"
REGEX1='s/\./,/g'
REGEX2='s/^\([^ ]*\) \([^ ]*\) \([^ ]*\) \([^ ]*\) \([^ ]*\)/\4 $BYTES_TOTALI \5 \1 \2 \3/'

if [ ! -f "$FILE_NAME" ]; then
  echo "Running a test with netperf"
  echo -e "t.time[s]\tbytes_transferred[bytes]\tthroughout [Mbps]\tTX buffer size [bytes]\tRX buffer size [bytes]\tmss [bytes]" > "$FILE_NAME"
fi

# Running a test with iperf
for i in $(seq 1 $REPETITIONS); do
	read C1 C2 C3 C4 C5 <<< $(netperf -H "$IP_DEST" -p 9002 -l -"$BYTES_TOTALI" -P 0 -- -m 1448)
	echo "$C4 $BYTES_TOTALI $C5 $C1 $C2 $C3" | sed "$REGEX1" | tr ' ' '\t' >> "$FILE_NAME"
done

