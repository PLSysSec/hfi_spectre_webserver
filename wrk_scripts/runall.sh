#!/bin/bash

# where is wrk?
WRK=../../wrk/wrk

# how many simultaneous connections
CONNECTIONS=100

# how many threads to use for requests
THREADS=10

# test duration
DURATION=60s

# timeout for individual requests
TIMEOUT=30s

mkdir -p results

if [ "$1" == "sfi" ]; then
  protections=("stock" "spectre_sfi_aslr" "spectre_sfi_full")
elif [ "$1" == "cet" ]; then
  protections=("stock" "spectre_cet_aslr" "spectre_cet_full")
else
  echo "Expected argument to be either 'sfi' or 'cet'"
  exit 1
fi

for protection in ${protections[@]}; do
  $WRK -c $CONNECTIONS -t $THREADS -d $DURATION --timeout $TIMEOUT -s ./html_template.lua "http://localhost:8000" -- $protection
  sleep 3
  $WRK -c $CONNECTIONS -t $THREADS -d $DURATION --timeout $TIMEOUT -s ./jpeg_resize_c.lua "http://localhost:8000" -- $protection
  sleep 3
  $WRK -c $CONNECTIONS -t $THREADS -d $DURATION --timeout $TIMEOUT -s ./xml_to_json.lua "http://localhost:8000" -- $protection
  sleep 3
  $WRK -c $CONNECTIONS -t $THREADS -d $DURATION --timeout $TIMEOUT -s ./msghash_check_c.lua "http://localhost:8000" -- $protection
  sleep 3
  $WRK -c $CONNECTIONS -t $THREADS -d $DURATION --timeout $TIMEOUT -s ./tflite.lua "http://localhost:8000" -- $protection
  sleep 3
done
