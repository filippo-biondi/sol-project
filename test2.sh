./bin/server config/configtest2.txt &

SERVER_PID=$!

bin/client -p -t 200 -f socket -W src/client.c,src/server.c  -r src/client.c,src/server.c -d config


bin/client -p -t 200 -f socket -w include,n=0  -R n=0 -d config

bin/client -p -t 200 -f socket -w src,n=0  -R n=0 -d config

bin/client -p -t 200 -f socket -l src/client.c -c src/client.c


bin/client -p -t 1000 -f socket -l src/server.c -u src/server.c 

bin/client -p -t 0 -f socket -l src/server.c

kill -1 ${SERVER_PID}
