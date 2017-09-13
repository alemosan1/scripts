# 1) set VM to 1GB
./set1GB

# 2) go to second terminal and launch the Database with 1GB of available memory for cache
cd /root
./launchdb.sh

# 3)Go back to the first terminal. Now fill up the database with 200000 entries of 4K each (same as private keys of the application)
cd /root
./fillup 4000 200000

# 4)launch redis-benchmark command to show performance

cd root/redis/src

redis-benchmark -p 7379 -t get -n 200000 -c 20  -r 200000

# 5) Now add to the database  500000 entries ( 700000 in total) 
./fillup 4000 500000 -c

# 6) launch redis-benchmark again to show performance

cd root/redis/src

redis-benchmark -p 7379 -t get -n 200000 -c 20  -r 700000


# COMMENT: In this momment the application would start its scale-up proccess. For demo's purpose we set the VM and the database cache size manually.

# 7) Now we change the memory of the VM to 3GB

./set3GB

# 8) Also we augment the memory available of our database to 3GB

cd root/
./augment

# 9) launch redis-benchmark again to show performance improvement

cd root/redis/src

redis-benchmark -p 7379 -t get -n 200000 -c 20  -r 700000

