./bin/server config/configtest2.txt &

SERVER_PID=$!

./bin/client -p -f socket -W random_file/rf1.txt -l random_file/rf1.txt
#write rf1.txt on the server and acquire lock on it

./bin/client -p -t 2000 -f socket -l random_file/rf1.txt -W random_file/rf2.txt -u random_file/rf1.txt &
#acquire the lock on rf1.txt, write rf2.txt and release lock on rf1.txt (waiting 1 second between request)

sleep 0.5
#sleep prevent the next client to be served before precedent client (this would cause stiil a correct output but upredictable and difficoult to read)

./bin/client -p -t 200 -f socket -r random_file/rf1.txt -W random_file/rf1.txt
#attempt to read a locked file so have to wait untill precedent client relese the lock and then try to write it (fail because file already exist)

./bin/client -p -f socket -w random_file,n=2
#write 2 files from random_file and its subdirectory random_file2

./bin/client -p -t 2000 -f socket -l random_file/rf2.txt -u random_file/rf2.txt &
#acquire lock ion rf2.txt and release it after 2 seconds

sleep 0.5
#same reason of other sleep

./bin/client -p -f socket -R -d dest
#read all file saved in the server and save them in dest (rf2.txt should not be in dest because is locked)

./bin/client -p -f socket -w random_file 
#write all files from random_file and it's subdirectory random_file2, this will cause the replacement algorithm to run multiple times (not all files will be written because some are too big)

./bin/client -p -t 500 -f socket -c random_file/random_file2/rf19 -c random_file/random_file2/rf20
#remove rf19 and rf20 from the server (not guaranteed to succeed because file are not written in a specific order so they could have been replaced)
./bin/client -p -f socket -R -d dest/dest2
#read all file saved in the server and save them in dest2 (this shoud match with the file list that server print on exit after reciving the signal, exept for rf2.txt whitch is locked)

kill -1 ${SERVER_PID}
#server exit only after lock on rf2 has been released (if signal would have been a SIGINT or SIGQUIT server woulbe be exited immediatly)
