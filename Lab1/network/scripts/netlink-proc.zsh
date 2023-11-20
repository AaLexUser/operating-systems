#!/bin/zsh
for i in {1..10..1};do
    sudo stress-ng --netlink-proc 5 --netlink-task 100 --metrics --timeout 30 --stdout |                                                   
    awk -v var="$i" '/[\[\]\d]+[ ]+netlink-proc/{printf "netlink;"$5";"$6";"$7";"$8";"$9";"$10";"$11"\n"}'
done
for i in {1..10..1};do
    sudo stress-ng --netlink-proc 5  --metrics --timeout 30 --stdout |
    awk -v var="$i" '/[\[\]\d]+[ ]+netlink-proc/{printf "empty;"$5";"$6";"$7";"$8";"$9";"$10";"$11"\n"}'
done