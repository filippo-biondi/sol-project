./bin/server config/configtest2.txt &

SERVER_PID=$!

./bin/client -p -f socket -W random_file/rf1.txt -l random_file/rf1.txt
#write rf1.txt on the server and acquire lock on it

./bin/client -p -t 1000 -f socket -l random_file/rf1.txt -W random_file/rf2.txt -u random_file/rf1.txt &
#acquire the lock on rf1.txt, write rf2.txt and release lock on rf1.txt (waiting 1 second between request)

./bin/client -p -f socket -r random_file/rf1.txt
#attempt to read a locked file so have to wait untill precedent client relese the lock

sleep 2

./bin/client -p -f socket -w random_file,n=2
#write 2 files from random_file and it's subdirectory random_file2

./bin/client -p -t 2000 -f socket -l random_file/rf2.txt -u random_file/rf2.txt &
#remove rf1.txt (could fail if rf2.txt has been removed by replacement algorithm)

sleep 1

./bin/client -p -f socket -R -d dest
#read all file saved in the server and save them in dest (rf2.txt should not be in dest because is locked)

./bin/client -p -f socket -w random_file 
#write all files from random_file and it's subdirectory random_file2

./bin/client -p -f socket -R -d dest/dest2
#read all file saved in the server and save them in dest2

kill -1 ${SERVER_PID}

sleep 1
