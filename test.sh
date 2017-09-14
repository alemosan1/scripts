#!/bin/bash

./fillup 4500 250000

cd redis/src

redis-benchmark -p 7379 -t get -n 250000 -c 20  -r 250000

cd ../../scaleuplib_C
./build.sh
./scaleupapi

cd ..
#Now we are going to augment the cache memory of our BBDD

./augment 2000000000

redis-benchmark -p 7379 -t get -n 250000 -c 20  -r 250000

echo "OK"
