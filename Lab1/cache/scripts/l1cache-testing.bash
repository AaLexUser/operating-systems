#!/bin/zsh
for i in {1..16..1}; do
    echo "run with l1cache=$i"
    a=`stress-ng --l1cache $i --metrics --timeout 2`
    echo "${a}" | cut -c 214-405
done