#!/bin/bash
# Version: v0.1
# Copyright 2019,2025 (@) Sebyone Srl
# Author: Sebastiano Meduri s.meduri@sebyone.it 
#
# Scope: network testing using dperf (https://github.com/sebyone/dperf)
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


# Case_1:
# defines an underlay/overlay test using Ipv4 and DaaS.INET4
# daas node is configurable by "daas_inet4.ini" located in the model folder
# The model to run is defined with the scripts (run_*) located in folder TEST_MODEL_FOLDER


source models/utils.sh

if [ $# -le 1 ]; then
  echo "Use: $0 <loopback_addr> <samples> <output_folder_name>"
  exit 1
fi

export SCRIPT_BASE=$(pwd)

echo "Base script: $SCRIPT_BASE"

# Level 0 script
# //
LOOPBACK_ADDRESS=$1
SAMPLES=$2
OUTPUT_NAME=$3
OUTPUT_FOLDER="results/$3"


if [ ! -d "$SCRIPT_BASE/results" ]; then
  mkdir "$SCRIPT_BASE/results"
fi


if [ ! -d "$OUTPUT_FOLDER" ]; then
  mkdir $OUTPUT_FOLDER
else
  rm -R $OUTPUT_FOLDER
  mkdir $OUTPUT_FOLDER
  # deletes all files in folder...
fi

# Model settings
# //
TEST_MODEL_FOLDER="overlay_ipv4" 

# Environment settings
# //
# // disable WiFi
# // enable eth0


# runs model for each cluster we need

./models/run_test.sh $TEST_MODEL_FOLDER $LOOPBACK_ADDRESS 1k $SAMPLES "$OUTPUT_FOLDER"
./models/run_test.sh $TEST_MODEL_FOLDER $LOOPBACK_ADDRESS 10k $SAMPLES "$OUTPUT_FOLDER"
./models/run_test.sh $TEST_MODEL_FOLDER $LOOPBACK_ADDRESS 100k $SAMPLES "$OUTPUT_FOLDER"
./models/run_test.sh $TEST_MODEL_FOLDER $LOOPBACK_ADDRESS 500k $SAMPLES "$OUTPUT_FOLDER"


# Environment reset
# //
# // enable WiFi
# // disable eth0


# Merge all csv files located in OUTPUT_FOLDER
# //
DATA_FILE=$(date +"%d%B%H%M%S")
merge_files $OUTPUT_FOLDER "${SCRIPT_BASE}/tool_overlay_ipv4_${OUTPUT_NAME}_${SAMPLES}_${DATA_FILE}.csv"
