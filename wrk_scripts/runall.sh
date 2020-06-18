#!/bin/bash

# where are various executables?
WRK=../../wrk/wrk
SPECTRESFI_WEBSERVER=../../spectresfi_webserver
SFI_SERVER=$SPECTRESFI_WEBSERVER/target/release/spectresfi_webserver
CET_SERVER=$SPECTRESFI_WEBSERVER/target-cet/release/spectresfi_webserver
TESTFIB=$SPECTRESFI_WEBSERVER/spectre_testfib.sh

# how many simultaneous connections
CONNECTIONS=100

# how many threads to use for requests
THREADS=10

# test duration for each test
DURATION_HTML=60s
DURATION_JPEG=3m
DURATION_XML=60s
DURATION_HASH=2m
DURATION_ML=5m

# timeout for individual requests
TIMEOUT=60s

mkdir -p results

# run these with the SFI server
sfi_protections=("stock" "spectre_sfi_aslr" "spectre_sfi_full")
# run these with the CET server
cet_protections=("spectre_cet_aslr" "spectre_cet_full")

# This ensures that if this script gets Ctrl-C, the background server process will be killed
trap "exit" INT TERM ERR
trap "kill 0" EXIT

launch_server() {
  if [ "$1" == "sfi" ]; then
    $SFI_SERVER &
  elif [ "$1" == "cet" ]; then
    $CET_SERVER &
  else
    echo "launch_server expected either parameter 'sfi' or 'cet'"
    exit 1
  fi
  server_pid=$!
  sleep 3
}

run_tests() {
  protection=$2

  echo "----------------------------"
  echo "Running tests for $protection"
  echo "----------------------------"

  echo
  echo "html_template"
  echo
  launch_server $1
  $TESTFIB $protection
  sleep 1
  $WRK -c $CONNECTIONS -t $THREADS -d $DURATION_HTML --timeout $TIMEOUT -s ./html_template.lua "http://localhost:8000" -- $protection
  kill $server_pid
  wait $server_pid

  echo
  echo "jpeg_resize_c"
  echo
  launch_server $1
  $TESTFIB $protection
  sleep 1
  $WRK -c $CONNECTIONS -t $THREADS -d $DURATION_JPEG --timeout $TIMEOUT -s ./jpeg_resize_c.lua "http://localhost:8000" -- $protection
  kill $server_pid
  wait $server_pid

  echo
  echo "xml_to_json"
  echo
  launch_server $1
  $TESTFIB $protection
  sleep 1
  $WRK -c $CONNECTIONS -t $THREADS -d $DURATION_XML --timeout $TIMEOUT -s ./xml_to_json.lua "http://localhost:8000" -- $protection
  kill $server_pid
  wait $server_pid

  echo
  echo "msghash_check_c"
  echo
  launch_server $1
  $TESTFIB $protection
  sleep 1
  $WRK -c $CONNECTIONS -t $THREADS -d $DURATION_HASH --timeout $TIMEOUT -s ./msghash_check_c.lua "http://localhost:8000" -- $protection
  kill $server_pid
  wait $server_pid

  echo
  echo "tflite"
  echo
  launch_server $1
  $TESTFIB $protection
  sleep 1
  $WRK -c $CONNECTIONS -t $THREADS -d $DURATION_ML --timeout $TIMEOUT -s ./tflite.lua "http://localhost:8000" -- $protection
  kill $server_pid
  wait $server_pid
}

for protection in ${sfi_protections[@]}; do
  run_tests sfi $protection
done

for protection in ${cet_protections[@]}; do
  run_tests cet $protection
done

