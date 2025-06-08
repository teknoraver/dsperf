#!/bin/bash

source ../utils.sh

#######################################################################################

# Check paramters
if [ $# -ne 4 ]; then
  echo "Use: $0 <IP_destination> <total_bytes> <samples> <filename_csv>"
  echo "Example: $0 192.168.1.1 1M folder data.csv"
  exit 1
fi

IP_DEST=$1
BYTES_TOTALI=$(convert_to_bytes $2)
REPETITIONS=$3
FILE_NAME="$4_dperf.csv"
###################################
REGEX2=#'s/\./,/g'
REGEX1='s/^\([^,]*\),[^,]*,[^,]*,[^,]*,[^,]*,/\1,/'



if [ ! -f "$FILE_NAME" ]; then
  echo "Running a test with dperf"
  dperf --underlay -s "$IP_DEST":5002 --blocksize "$BYTES_TOTALI" -y 1 -n 1 | sed 's/\./,/g' >> "tmp/$FILE_NAME"
else
  dperf --underlay -s "$IP_DEST":5002 --blocksize "$BYTES_TOTALI" -y 0 -n 1 | sed 's/\./,/g' >> "tmp/$FILE_NAME"
fi

