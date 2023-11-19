sudo stress-ng --zlib 8 --fork-vm --metrics --timeout 30 --stdout --vmstat 1|
awk '/vmstat/{printf $8"\n"}'