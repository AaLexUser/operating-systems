#!/bin/zsh
echo "nice;bogo_ops;real_time;usr_time;sys_time;bogo_ops/s_real;bogo_ops/s_user+sys;CPU_used_per"
for i in {-20..19..1}; do
    sudo nice -n $i stress-ng --cpu 0 --cpu-method fft --metrics -t 2m --stdout | 
    awk -v var="$i" '/[\[\]\d]+[ ]+cpu/{printf var";"$5";"$6";"$7";"$8";"$9";"$10";"$11"\n"}'
done