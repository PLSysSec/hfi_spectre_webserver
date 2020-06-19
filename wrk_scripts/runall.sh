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
DURATION_ECHO=60s
DURATION_HTML=60s
DURATION_JPEG=3m
DURATION_XML=60s
DURATION_HASH=2m
DURATION_ML=15m

# timeout for individual requests
TIMEOUT=60m

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

run_wrk() {
  protection=$1
  lua=$2
  duration=$3
  $WRK -c $CONNECTIONS -t $THREADS -d $duration --timeout $TIMEOUT -s $lua "http://localhost:8000" -- $protection
}

run_test() {
  sfi_or_cet=$1
  protection=$2
  lua=$3
  duration=$4

  echo
  echo $protection :
  echo

  launch_server $sfi_or_cet
  $TESTFIB $protection
  sleep 1
  run_wrk $protection $lua $duration

  echo
  echo "Killing server..."
  kill $server_pid
  wait $server_pid
  echo "Killed server"
}

run_tests() {
  workload=$1
  duration=$2

  lua=./$workload.lua

  echo
  echo "----------------------------"
  echo "Running tests for $workload"
  echo "----------------------------"
  echo

  for protection in ${sfi_protections[@]}; do
    run_test sfi $protection $lua $duration
  done
  for protection in ${cet_protections[@]}; do
    run_test cet $protection $lua $duration
  done

  echo
  echo "----------------------------"
  echo "Done with tests for $workload"
  echo "----------------------------"
  echo
}

run_tests echo_server     $DURATION_ECHO
run_tests html_template   $DURATION_HTML
run_tests jpeg_resize_c   $DURATION_JPEG
run_tests xml_to_json     $DURATION_XML
run_tests msghash_check_c $DURATION_HASH
run_tests tflite          $DURATION_ML
