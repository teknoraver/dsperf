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


if [ $# -le 1 ]; then
  echo "Use: $0 <folder>"
  exit 1
fi

DIRECTORY=$1

echo $DIRECTORY
# Check that the path exists and is a directory
if [ ! -d "$DIRECTORY" ]; then
  echo "Error: $DIRECTORY is not a valid directory"
  exit 2
fi

echo "Directory trovate in: $DIRECTORY"

list_directory_files() {
  local dir="$1"
  file_array=()

  # Check if a valid directory was provided
  if [ -z "$dir" ] || [ ! -d "$dir" ]; then
    echo "Error: '$dir' is not a valid directory."
    return 1
  fi

  for file in "$dir"/*; do
    [ -f "$file" ] && file_array+=("$(basename "$file")")
  done
    
  paste $dir/${file_array[1]%_*}_iperf.csv <(cut -f1,3 $dir/${file_array[1]%_*}_netperf.csv) $dir/${file_array[1]%_*}_nuttcp.csv <(cut -f12,13 $dir/${file_array[1]%_*}_dperf.csv) > "$(basename "$dir")_$(basename "$(dirname "$dir")")".csv
}

# Loop over directories only
for dir in "$DIRECTORY"/*/; do
  [ -d "$dir" ] && list_directory_files $dir  
done