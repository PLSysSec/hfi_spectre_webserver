#!/bin/bash

# where is wrk?
WRK=../../wrk/wrk

# how many simultaneous connections
CONNECTIONS=100

# how many threads to use for requests
THREADS=10

# test duration for each test
DURATION_HTML=60s
DURATION_JPEG=60s
DURATION_XML=60s
DURATION_HASH=2m
DURATION_ML=3m

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
  $WRK -c $CONNECTIONS -t $THREADS -d $DURATION_HTML --timeout $TIMEOUT -s ./html_template.lua "http://localhost:8000" -- $protection
  sleep 3
  $WRK -c $CONNECTIONS -t $THREADS -d $DURATION_JPEG --timeout $TIMEOUT -s ./jpeg_resize_c.lua "http://localhost:8000" -- $protection
  sleep 3
  $WRK -c $CONNECTIONS -t $THREADS -d $DURATION_XML --timeout $TIMEOUT -s ./xml_to_json.lua "http://localhost:8000" -- $protection
  sleep 3
  $WRK -c $CONNECTIONS -t $THREADS -d $DURATION_HASH --timeout $TIMEOUT -s ./msghash_check_c.lua "http://localhost:8000" -- $protection
  sleep 3
  $WRK -c $CONNECTIONS -t $THREADS -d $DURATION_ML --timeout $TIMEOUT -s ./tflite.lua "http://localhost:8000" -- $protection
  sleep 3
done
