#!/bin/zsh
iostat -xzdtk -o JSON 1 sda >> ./io-uring.json &
iostat_pid=$!
sudo stress-ng --io-uring 8 --metrics --timeout 30m  > /dev/null &
stress_ng_pid=$!  # Get the process ID of stress-ng
wait $stress_ng_pid 
kill $iostat_pid