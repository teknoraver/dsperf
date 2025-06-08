#!/bin/bash

#Convert k, M, G, T suffixes
convert_to_bytes(){
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

merge_files() {

  DIRECTORY=$1

  # Check that the path exists and is a directory
  if [ ! -d "$DIRECTORY" ]; then
    echo "Error: $DIRECTORY is not a valid directory"
    exit 2
  fi

  file_array=()

  for file in "$DIRECTORY"/*; do
    #[ -f "$file" ] && file_array+=("$(basename "$file")")
    [ -f "$file" ] && file_array+="$file "
  done

  echo "${file_array[@]}"

  paste ${file_array[@]} > new_file.csv

}