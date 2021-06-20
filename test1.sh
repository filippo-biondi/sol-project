valgrind --leak-check=full ./bin/server config/configtest1.txt &

SERVER_PID=$!

./bin/client -p -t 200 -f socket -W random_file/rf1.txt,random_file/rf2.txt,random_file/rf3.txt
#write rf1.txt rf2.txt, rf3.txt on the server

./bin/client -p -t 200 -f socket -r random_file/rf1.txt -d dest
#read rf1.txt from the server and save it in dest

./bin/client -p -t 200 -f socket -c random_file/rf1.txt
#remove rf1.txt from server

./bin/client -p -t 200 -f socket -r random_file/rf1.txt -d dest
#attempt to read a removed file

./bin/client -p -t 200 -f socket -w random_file
#write all file in random_file and it's subdirectory random_file2 (some have been already written in precedence)

./bin/client -p -t 200 -f socket -R -d dest/dest2
#read all file saved in the server and save them in dest2

./bin/client -p -t 200 -f socket -R -d dest/dest3
#try to read all file saved in the server and save them in dest3 and fail because dest3 doesn't exist 

./bin/client -p -t 200 -f socket -l random_file/rf2.txt -u random_file/rf2.txt
#acquire and release lock on rf2.txt

./bin/client -p -t 200 -f socket -l random_file/rf3.txt
#acquire lock on rf3.txt without explicitly releasing it (will be automatically released by server on client disconnection)

./bin/client -p -t 200 -f socket -r random_file/rf3.txt
#read rf3.txt (possible because server releasd the lock)

kill -1 ${SERVER_PID}
#SIGHUP signal
