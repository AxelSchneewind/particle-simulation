#!/bin/bash



for N in {0..12..4}
do
    echo $N
    ./fmtest.o $N >> /dev/null &
    ./fmtest.o $((N+1)) >> /dev/null &
    ./fmtest.o $((N+2)) >> /dev/null &
    ./fmtest.o $((N+3)) >> /dev/null &
    wait
done

for M in {16..48..16}
do
    echo $M
    ./fmtest.o $M >> /dev/null &
    ./fmtest.o $((M+4)) >> /dev/null &
    ./fmtest.o $((M+8)) >> /dev/null &
    ./fmtest.o $((M+12)) >> /dev/null &
    wait
done

for M in {64..96..32}
do
    echo $M
    ./fmtest.o $M >> /dev/null &
    ./fmtest.o $((M + 8)) >> /dev/null &
    ./fmtest.o $((M + 16)) >> /dev/null &
    ./fmtest.o $((M + 24)) >> /dev/null &
    wait
done

for M in {128..512..64}
do
    echo $M
    ./fmtest.o $M >> /dev/null &
    ./fmtest.o $((M + 16)) >> /dev/null &
    wait
    ./fmtest.o $((M + 32)) >> /dev/null &
    ./fmtest.o $((M + 48)) >> /dev/null &
    wait
done



./timings.py

./timings.py plot pls


