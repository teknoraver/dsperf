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
  echo $#
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

FILE_NAME="$1_$3_${DATA_FILE}_netperf.csv"
##########################################

REGEX1='s/\./,/g'
REGEX2='s/^\([^ ]*\) \([^ ]*\) \([^ ]*\) \([^ ]*\) \([^ ]*\)/\4 $BYTES_TOTALI \5 \1 \2 \3/'

if [ ! -f "$FILE_NAME" ]; then
  #echo "Running a test with netperf"
  echo "$FILE_NAME $FILE_NAME $FILE_NAME $FILE_NAME $FILE_NAME $FILE_NAME" | tr ' ' '\t' > "$OUTPUT_FOLDER/$FILE_NAME"
  echo -e "transfer_time[s] bytes_transferred[bytes] throughout[Mbps] TX_buffer_size[bytes] RX_buffer_size[bytes] mss[bytes]" | tr ' ' '\t' >> "$OUTPUT_FOLDER/$FILE_NAME"
fi

# Running a test with netperf
for i in $(seq 1 $SAMPLES); do
	read C1 C2 C3 C4 C5 <<< $(netperf -H "$IP_DEST" -p 9002 -l -"$BYTES_TOTALI" -P 0 -- -m 1448)
	echo "$C4 $BYTES_TOTALI $C5 $C1 $C2 $C3" | sed "$REGEX1" | tr ' ' '\t' >> "$OUTPUT_FOLDER/$FILE_NAME"
done
