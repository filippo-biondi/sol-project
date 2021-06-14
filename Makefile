CC		=  gcc
CFLAGS	  += -std=c99 -Wall -g
INCLUDES	= -I ./include
LDFLAGS 	= -Wl,-rpath,./lib -L ./lib

# aggiungere qui altri targets
TARGETS		= client

.PHONY: all clean cleanall
.SUFFIXES: .c .h

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
  

  
all		: $(TARGETS)


clean		: 
	rm -f $(TARGETS)
cleanall	: clean
	\rm -f *.o *~



