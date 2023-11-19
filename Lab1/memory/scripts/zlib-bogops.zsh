#!/bin/zsh
echo "zlib;bogo_ops;real_time;usr_time;sys_time;bogo_ops/s_real;bogo_ops/s_user+sys;CPU_used_per"
for i in {1..8192..1};do
    sudo stress-ng --zlib $i --metrics --timeout 30 --stdout| 
    awk -v var="$i" '/[\[\]\d]+[ ]+zlib/{printf var";"$5";"$6";"$7";"$8";"$9";"$10";"$11"\n"}'
done 