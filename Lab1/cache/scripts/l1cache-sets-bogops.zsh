#!/bin/zsh
for i in {1..1000..1}; do
    a=`stress-ng --l1cache 8 --l1cache-sets $i --metrics --timeout 30 --stdout`
    echo "$i ${a:521:652}"
done