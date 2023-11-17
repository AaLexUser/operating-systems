#!/bin/zsh
for i in {1..100..1}; do
    a=`stress-ng --l1cache $i --metrics --timeout 2 --stdout`
    echo "$i ${a:521:652}"
done