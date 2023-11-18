#!/bin/zsh
echo "sched;bogo_ops;real_time;usr_time;sys_time;bogo_ops/s_real;bogo_ops/s_user+sys;CPU_used_per"
for i in {1..8192..1};do
    sudo stress-ng --io-uring $i --metrics --timeout 1m --stdout| 
    awk -v var="$i" '/[\[\]\d]+[ ]+io-uring/{printf var";"$5";"$6";"$7";"$8";"$9";"$10";"$11"\n"}'
done 