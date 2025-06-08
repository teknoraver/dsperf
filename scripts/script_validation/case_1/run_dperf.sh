#!/bin/bash
# dperf script v-1.0.0

source utils.sh

#######################################################################################

# Check paramters
if [ $# -ne 5 ]; then
  echo "Use: $0 <scripts_folder_name> <loopback_addr> <data_block_size> <samples> <output_folder_name>"
  echo "Example: $0 192.168.1.1 1M folder data.csv"
  exit 1
fi

IP_DEST=$2
BYTES_TOTALI=$(convert_to_bytes $3)
SAMPLES=$4
OUTPUT_FOLDER=$5

DATA_FILE=$(date +"%d%B%H%M%S")

FILE_NAME="$1_$3_$DATA_FILE_dperf.csv"
###################################
REGEX2=#'s/\./,/g'
REGEX1='s/^\([^,]*\),[^,]*,[^,]*,[^,]*,[^,]*,/\1,/'


# Running a test with iperf
for i in $(seq 1 $SAMPLES); do
  if [ ! -f "$OUTPUT_FOLDER/$FILE_NAME" ]; then
    echo "Running a test with dperf"
    dperf --underlay -s "$IP_DEST":5002 --blocksize "$BYTES_TOTALI" -y 1 -n 1 | sed 's/\./,/g' > "$OUTPUT_FOLDER/$FILE_NAME"
  else
    dperf --underlay -s "$IP_DEST":5002 --blocksize "$BYTES_TOTALI" -y 0 -n 1 | sed 's/\./,/g' >> "$OUTPUT_FOLDER/$FILE_NAME"
  fi
done

