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


# Script level 1


if [ $# -le 1 ]; then
  echo "Use: $0 <scripts_folder_name> <loopback_addr> <data_block_size> <samples> <output_folder_name>"
  exit 1
fi


SCRIPTS_FOLDER=$1

OPTIONS=("${@:1}")


if [ -d $FOLDER ]; then
	echo "########################################################"
	echo "#### Execution Scripts in forlder '$SCRIPTS_FOLDER' ####"
	for script in $SCRIPT_BASE/models/$SCRIPTS_FOLDER/*;
	do
		if [ -f "$script" ] && [ -x "$script" ]; then
			echo "#### Execution Scripts '$script' ####" 
			# run_<tool>.sh  <scripts_folder_name> <loopback_addr> <data_block_size> <samples> <output_folder_name> 
			$script ${OPTIONS[@]}
		fi
	done
	echo "########################################################"
else
	echo "#### Folder not found ####"
fi
