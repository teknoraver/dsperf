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

# Level 2 script


# Eseguo test sulla underlay WiFi


if [ $# -le 1 ]; then
  echo "Use: $0 <loopback_addr> <samples> <output_folder_name>"
  exit 1
fi

SCRIPT_FOLDER="case_1"
LOOPBACK_ADDRESS=$1
SAMPLES=$2
OUTPUT_FOLDER=$3

if [ ! -d "$OUTPUT_FOLDER" ]; then
  mkdir $OUTPUT_FOLDER
fi


# Settare ambiente di sistema


# execute all run_test* units in folder <tests_dir>
# Test underlay WiFi
./run_test.sh  $SCRIPT_FOLDER $LOOPBACK_ADDRESS $SAMPLES 100k "../$OUTPUT_FOLDER"
./run_test.sh  $SCRIPT_FOLDER $LOOPBACK_ADDRESS $SAMPLES 5M "../$OUTPUT_FOLDER"
./run_test.sh  $SCRIPT_FOLDER $LOOPBACK_ADDRESS $SAMPLES 100M "../$OUTPUT_FOLDER"
./run_test.sh  $SCRIPT_FOLDER $LOOPBACK_ADDRESS $SAMPLES 300M "../$OUTPUT_FOLDER"


# Merge all files
#merge_files.sh $OUTPUT_FOLDER
