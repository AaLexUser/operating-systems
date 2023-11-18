#!/bin/zsh
echo "sched;bogo_ops;real_time;usr_time;sys_time;bogo_ops/s_real;bogo_ops/s_user+sys;CPU_used_per"
schedulers=("batch" "deadline" "fifo" "idle" "other" "rr")
for sched in "${schedulers[@]}"; do
    sudo nice -n -20 stress-ng --cpu 0 --cpu-method fft --metrics -t 30 --stdout --sched $sched| 
    awk -v var="$sched" '/[\[\]\d]+[ ]+cpu/{printf var";"$5";"$6";"$7";"$8";"$9";"$10";"$11"\n"}'
done