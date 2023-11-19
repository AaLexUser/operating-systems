#!/bin/zsh
for i in {1..9..1};do
    for j in {1..10..1};do
        a=`sudo stress-ng --zlib 1 --zlib-mem-level $i --metrics --timeout 10 --stdout| 
        awk '/ratio/'`
        echo "$i;${a:73:95}"
    done
done 