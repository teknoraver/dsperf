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


# Level 0 script

source utils.sh

if [ $# -ne 3 ]; then
	echo "Numero di parametri: $#"
  echo "Use: $0 <loopback_addr> <samples> <output_folder_name>"
  exit 1
fi

SCRIPTS_FOLDER="ipv4"
LOOPBACK_ADDRESS=$1
SAMPLES=$2
OUTPUT_FOLDER=$3

if [ ! -d $OUTPUT_FOLDER ]; then
	mkdir $OUTPUT_FOLDER
fi

./run_test.sh  $SCRIPTS_FOLDER $LOOPBACK_ADDRESS 100k $SAMPLES "$OUTPUT_FOLDER"
./run_test.sh  $SCRIPTS_FOLDER $LOOPBACK_ADDRESS 5M $SAMPLES "$OUTPUT_FOLDER"
#./run_test.sh  $SCRIPTS_FOLDER $LOOPBACK_ADDRESS 100M $SAMPLES "$OUTPUT_FOLDER"
#./run_test.sh  $SCRIPTS_FOLDER $LOOPBACK_ADDRESS 300M $SAMPLES "$OUTPUT_FOLDER"


merge_files $OUTPUT_FOLDER