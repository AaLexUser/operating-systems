#!/bin/zsh
echo "pipe;bogo_ops;real_time;usr_time;sys_time;bogo_ops/s_real;bogo_ops/s_user+sys;CPU_used_per"
for i in {100000..100000000000..10000};do
    sudo stress-ng --pipe 4 --pipe-ops $i --metrics --stdout| 
    awk -v var="$i" '/[\[\]\d]+[ ]+pipe/{printf var";"$5";"$6";"$7";"$8";"$9";"$10";"$11"\n"}'
done