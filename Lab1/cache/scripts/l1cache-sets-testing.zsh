#!/bin/bash
mpstat -P ALL 1 &
mpstat_pid=$!
for i in {1..16..1}; do
    echo "l1cache-sets=$i"
    stress-ng --l1cache 8 --l1cache-sets $i --metrics --timeout 30 > /dev/null &
    stress_ng_pid=$!  # Get the process ID of stress-ng
    wait $stress_ng_pid  # Wait for stress-ng to finish before moving to the next iteration
done
kill $mpstat_pid