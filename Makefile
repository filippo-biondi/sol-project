CC		=  gcc
CFLAGS	  += -std=c99 -Wall -g -pthread
INCLUDES	= -I ./include
LDFLAGS 	= -Wl,-rpath,./lib -L ./lib

# aggiungere qui altri targets
TARGETS		= ./bin/client ./bin/server

.PHONY: all clean cleandest cleanall test1 test2
.SUFFIXES: .c .h .a. so

all: $(TARGETS)

./bin/client: ./obj/client.o ./obj/execute_command.o ./lib/libi_conn.so ./lib/libconn_supp.so
	$(CC) $(CFLAGS) ./obj/client.o ./obj/execute_command.o -o $@ $(LDFLAGS) -li_conn -lconn_supp

./obj/client.o: ./src/client.c ./include/utils.h ./include/i_conn.h
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
	
./obj/execute_command.o: ./src/execute_command.c ./include/utils.h ./include/i_conn.h ./include/conn_supp.h
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
  
./obj/utils.o: ./src/utils.c ./include/utils.h
	$(CC) $(CFLAGS) $(INCLUDES) -c -fPIC -o $@ $<
	
./obj/i_conn.o: ./src/i_conn.c ./include/utils.h ./include/i_conn.h
	$(CC) $(CFLAGS) $(INCLUDES) -c -fPIC -o $@ $<
	
./obj/conn_supp.o: ./src/conn_supp.c ./include/utils.h ./include/i_conn.h ./include/conn_supp.h
	$(CC) $(CFLAGS) $(INCLUDES) -c -fPIC -o $@ $<

./lib/libi_conn.so: ./obj/i_conn.o
	$(CC) $(CFLAGS) -shared -o $@ $<

./lib/libconn_supp.so: ./obj/conn_supp.o
	$(CC) $(CFLAGS) -shared -o $@ $<

./bin/server: ./obj/server.o ./obj/worker_routine.o ./obj/server_op.o ./obj/signal_handler.o ./lib/libshared_queue.a ./lib/libicl_hash.a
	$(CC) $(CFLAGS) ./obj/server.o ./obj/worker_routine.o ./obj/server_op.o ./obj/signal_handler.o -o $@ -L ./lib -lshared_queue -licl_hash

./obj/server.o: ./src/server.c ./include/server_lib.h
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

./obj/worker_routine.o: ./src/worker_routine.c ./include/server_lib.h ./include/server_op.h
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

./obj/server_op.o: ./src/server_op.c ./include/server_op.h ./include/lock_lib.h
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
	
./obj/signal_handler.o: ./src/signal_handler.c ./include/server_lib.h
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

./obj/shared_queue.o: ./src/shared_queue.c ./include/shared_queue.h
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

./obj/icl_hash.o: ./src/icl_hash.c ./include/icl_hash.h
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

./lib/libshared_queue.a: ./obj/shared_queue.o
	ar rvs $@ $<

./lib/libicl_hash.a: ./obj/icl_hash.o
	ar rvs $@ $<
	
test1: $(TARGETS)
	chmod +x ./test2.sh
	./test1.sh
	
test2: $(TARGETS)
	chmod +x ./test2.sh
	./test2.sh


clean: 
	rm -f $(TARGETS)
	
cleandest:
	find ./dest -type f -delete

cleanall:
	rm -f ./obj/* ./lib/* ./bin/* *~
	find ./dest -type f -delete


