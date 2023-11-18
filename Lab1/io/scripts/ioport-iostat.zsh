#!/bin/zsh
iostat -xzdtk -o JSON 1 sda >> ./ioport-iostat.json &
iostat_pid=$!
sudo stress-ng --ioport 8 --metrics --timeout 30m  > /dev/null &
stress_ng_pid=$!  # Get the process ID of stress-ng
wait $stress_ng_pid 
kill $iostat_pid