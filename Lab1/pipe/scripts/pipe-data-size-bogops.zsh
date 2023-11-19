#!/bin/zsh
echo "pipe_data_size;bogo_ops;real_time;usr_time;sys_time;bogo_ops/s_real;bogo_ops/s_user+sys;CPU_used_per"
for i in {4..4096..16};do
    sudo stress-ng --pipe 4 --pipe-data-size $i --metrics --timeout 10 --stdout| 
    awk -v var="$i" '/[\[\]\d]+[ ]+pipe/{printf var";"$5";"$6";"$7";"$8";"$9";"$10";"$11"\n"}'
done 