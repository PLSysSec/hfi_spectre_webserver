#!/bin/bash

# where are various executables?
WRK=../../wrk/wrk
SPECTRESFI_WEBSERVER=../../spectresfi_webserver
SFI_SERVER=$SPECTRESFI_WEBSERVER/target/release/spectresfi_webserver
CET_SERVER=$SPECTRESFI_WEBSERVER/target-cet/release/spectresfi_webserver
TESTFIB=$SPECTRESFI_WEBSERVER/spectre_testfib.sh

# how many simultaneous connections
CONNECTIONS=100
ML_CONNECTIONS=20

# how many threads to use for requests
THREADS=10

# test duration for each test
DURATION_ECHO=60s
DURATION_HTML=60s
DURATION_JPEG=3m
DURATION_XML=60s
DURATION_HASH=2m
DURATION_ML=15m

# How long to run wrk for as warmup
WARMUP_DURATION=10s
# How long to wait between warmup and the real run (to let server get ready) (in seconds)
WARMUP_SLEEP=10
# special warmup_sleep for tflite
WARMUP_SLEEP_TFLITE=60

# timeout for individual requests
TIMEOUT=10m

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
    echo "!!!!!!!!Ignore module load failures below!!!!!!!!"
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
  conns=$4
  $WRK -c $conns -t $THREADS -d $duration --timeout $TIMEOUT -s $lua "http://localhost:8000" -- $protection
}

run_test() {
  sfi_or_cet=$1
  protection=$2
  lua=$3
  duration=$4
  conns=$5
  workload=$6

  echo
  echo "----------------------------"
  echo "Running tests for $workload: $protection"
  echo "----------------------------"
  echo

  echo
  echo $protection :
  echo

  launch_server $sfi_or_cet
  $TESTFIB $protection
  sleep 1

  # warmup
  run_wrk $protection $lua $WARMUP_DURATION $conns # these results will get overwritten
  sleep $WARMUP_SLEEP
  if [ "$lua" == "./tflite.lua" ]; then
    sleep $WARMUP_SLEEP_TFLITE
  fi

  run_wrk $protection $lua $duration $conns

  echo
  echo "Killing server..."
  kill $server_pid
  wait $server_pid
  echo "Killed server"
}

run_tests() {
  workload=$1
  duration=$2
  conns=$3

  lua=./$workload.lua

  echo
  echo "----------------------------"
  echo "Running tests for $workload"
  echo "----------------------------"
  echo

  for protection in ${sfi_protections[@]}; do
    run_test sfi $protection $lua $duration $conns $workload
  done
  for protection in ${cet_protections[@]}; do
    run_test cet $protection $lua $duration $conns $workload
  done

  echo
  echo "----------------------------"
  echo "Done with tests for $workload"
  echo "----------------------------"
  echo
}

# run_tests echo_server     $DURATION_ECHO $CONNECTIONS
run_tests html_template   $DURATION_HTML $CONNECTIONS
run_tests jpeg_resize_c   $DURATION_JPEG $CONNECTIONS
run_tests xml_to_json     $DURATION_XML  $CONNECTIONS
run_tests msghash_check_c $DURATION_HASH $CONNECTIONS
# run_tests tflite          $DURATION_ML   $ML_CONNECTIONS
