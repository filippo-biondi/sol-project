#include <server_lib.h>

void* signal_handler(void* arg) 
{
  sigset_t set;
  int sig;
	int r;
	int* terminate = ((struct handler_args*) arg)->terminate;
	
	sigemptyset(&set);
  sigaddset(&set, SIGINT); 
  sigaddset(&set, SIGQUIT);
  sigaddset(&set, SIGHUP); 
  
 r = sigwait(&set, &sig);  //wait for one of the 3 signal
 if (r != 0) 
 {
   errno = r;
   perror("Sigwait error");
   return NULL;
 }

 switch(sig) 
 {
   case SIGINT:
   case SIGQUIT:
     *terminate = -1;  //inform main thread that he has to terminate immediatly
     close(((struct handler_args*) arg)->pipe_fd);  //wake up main from select 
     return NULL;
     
   case SIGHUP:
     *terminate = 1;  //inform main thread to not accept new connection and to terminate when all client are disconnected
     close(((struct handler_args*) arg)->pipe_fd);  //wake up main from select 
    return NULL;
	}
	return NULL;
}
