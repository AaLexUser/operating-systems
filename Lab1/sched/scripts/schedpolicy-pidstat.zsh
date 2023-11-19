#!/bin/zsh
sudo stress-ng --schedpolicy 100 --metrics --timeout 10m &
sleep 30
ps aux | awk '/stress-ng-schedpolicy \[run\]/{print $2}' | head -n1 | xargs -I{} pidstat -R -p {} 1  > ./pidstat.txt
