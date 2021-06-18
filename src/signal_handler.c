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
  
  while(1)
  {
    r = sigwait(&set, &sig);
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
	      *terminate = -1;
	      close(((struct handler_args*) arg)->pipe_fd);
	      return NULL;
	      
	    case SIGHUP:
	      *terminate = 1;
	      close(((struct handler_args*) arg)->pipe_fd);
	      return NULL; 
	  }
  }   
}
