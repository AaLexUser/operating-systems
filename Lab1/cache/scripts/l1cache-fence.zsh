mpstat -P ALL 1 &
mpstat_pid=$!
stress-ng --l1cache 8 --l1cache-sets 1000 --cache-fence --metrics --timeout 30 > /dev/null &
stress_ng_pid=$!  # Get the process ID of stress-ng
wait $stress_ng_pid  # Wait for stress-ng to finish before moving to the next iteration
kill $mpstat_pid