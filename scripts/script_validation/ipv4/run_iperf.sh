#!/bin/bash
# Version: v0.1
# Copyright 2019,2025 (@) Sebyone Srl
# Author: Sebastiano Meduri s.meduri@sebyone.it 
#
# Scope: speed up network testing using dperf (https://github.com/sebyone/dperf)
#
# This Source Code Form is subject to the terms of the Mozilla Public License,v.2.0.
# You can obtain a copy of the MPL at https://mozilla.org/MPL/2.0/.
#
# Disclaimer of Warrant
# Covered Software is provided under this License on an "as is" basis, without warranty of any kind, either
# expressed, implied, or statutory, including, without limitation, warranties that the Covered  Software is
# free of defects, merchantable, fit for a particular purpose or non-infringing.
# The entire risk as to the quality and performance of the Covered Software is with You.  Should any Covered
# Software prove defective in any respect, You (not any Contributor) assume the cost of any necessary
# servicing, repair, or correction.
# This disclaimer of warranty constitutes an essential part of this License.  
# No use of any Covered Software is authorized under this License except under this disclaimer.

source utils.sh

################

# Check paramters
if [ $# -ne 5 ]; then
  echo "Use: $0 <scripts_folder_name> <loopback_addr> <data_block_size> <samples> <output_folder_name>"
  echo "Example: $0 wifi 192.168.1.1 1M 10 test-data"
  exit 1
fi

##########################################
IP_DEST=$2
BYTES_TOTALI=$(convert_to_bytes $3)
SAMPLES=$4
OUTPUT_FOLDER=$5
##########################################
DATA_FILE=$(date +"%d%B%H%M%S")

FILE_NAME="$1_$3_${DATA_FILE}_iperf.csv"
##########################################

REGEX1='s/^ *([0-9.]+) MB *\/ *([0-9.]+) sec *= *([0-9.]+) Mbps.*/\2 \1 \3/'

# Running a test with iperf
if [ ! -f "$FILE_NAME" ]; then
  #echo "Running a test with iperf"
  echo "$FILE_NAME $FILE_NAME" | tr ' ' '\t' > "$OUTPUT_FOLDER/$FILE_NAME"
  echo "transfer_time[s] throughout[MBps]" | tr ' ' '\t' >> "$OUTPUT_FOLDER/$FILE_NAME"
fi

# Running a test with iperf
for i in $(seq 1 $SAMPLES); do
  iperf -c "$IP_DEST" -p 5001 -n "$BYTES_TOTALI" -f M | tail -n 1 | awk '{split($3,a,"-"); durata=a[2]; banda=$7; print durata, banda}'| tr ' ' '\t' | sed 's/\./,/g' >> "$OUTPUT_FOLDER/$FILE_NAME"
done