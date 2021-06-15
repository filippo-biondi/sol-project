#include <utils.h>
#include <pthread.h>
#include <sys/select.h>
#include <shared_queue.h>

#define MUTEX_INT(mutex) \
if(pthread_mutex_init(&(mutex), NULL) != 0) \
  {                                       \
    perror("Mutex initalization error");  \
    exit(EXIT_FAILURE);                   \
  }                                       \

struct saved_file
{
  char* name;
  void* buf;
  int locked;
  waitng_queue;
};

struct shared_state
{
  int n_worker = 10;
  long int max_storage = 100000000;
  int max_n_file = 100;
  
  int max_reached_storage = 0;
  long int max_reached_n_file = 0;
  int n_replacement = 0;
  
  long int used_storage = 0;
  int n_saved_file;
  pthread_mutex_lock mutex;
  
  SharedQueue work_queue;
  int pipe_fd;
};



extern void* worker_routine(void* arg);

int main(int argc, char* argv[])
{

  struct shared_state param;
  char* socket_name;
  int fd_skt;
  struct sockaddr_un sa;
  fd_set set
  fd_set tmpset;
  int fd_max;
  int fd;
  int* fdPtr;
  int pipeFd[2];
  FILE* config;
  
  if(argc > 2)
  {
    printf("Invalid arguments\n")
    return 0;
  }
  
  if(argc == 2)
  {
    if((fd_config = fopen(argv[1], "r")) == NULL)
    {
      perror("Error in config file opening, default values used");
    }
    if(fscanf("N_WORKER=%d", &param.n_worker) != 1)
    {
      perror("Error in config file reading, default values for N_WORKER used");
    }
    if(fscanf("MAX_STORAGE=%Ld", &param.max_storage) != 1)
    {
      perror("Error in config file reading, default values for MAX_STORAGE used");
    }
    if(fscanf("MAX_N_FILE=%d", &param.max_n_file) != 1)
    {
      perror("Error in config file reading, default values for MAX_N_FILE used");
    }
    MALLOC(param.socket_name, NAME_MAX+1)
    if(fscanf("SOCKET_NAME=%s", param.socket_name) != 1)
    {
      strcpy(socket_name, "socket");
      perror("Error in config file reading, default values for SOCKET NAME used");
    }
  }
  MUTEX_INIT(param.mutex)
  
  intiQueue(&(param.work_queue);
  
  if(pipe(pipeFd) == -1)
  {
    perror("Error in pipe creating");
    exit(EXIT_FAILURE);
  }
  param.pipe_fd = pipeFd[1];
  
  pthread_t workers[param.n_worker];
  for(int i=0; i < n_worker; i++)
  {
    if(pthread_create(&(workers[i]), NULL, worker_routine, &param) != 0)
    {
      perror("Error in creating pool thread");
      if(i == 0)
      {
        exit(EXIT_FAILURE);
      }
      else
      {
        printf("Only %d thread in the pool\n", i);
        n_worker = i;
        break;
      }
    }
  }
  
  strncpy(sa.sun_path, SOCKNAME, NAME_MAX);  
  free(socket_name);
  sa.sun_family = AF_UNIX;
  if((fd_skt = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
  {
    perror("Error in socket creating");
    exit(EXIT_FAILURE);
  }
  
  if(bind(fd_skt, (struct sockaddr *)&sa, sizeof(sa)) == -1)
  {
    perror("Error in socket binding");
    exit(EXIT_FAILURE);
  }
  if(listen(fd_skt, SOMAXCONN) == -1)
  {
    perror("Error in socket listening");
    exit(EXIT_FAILURE);
  }
  
  FD_ZERO(&set);
  FD_ZERO(&tempset);
  FD_SET(fd_skt, &set);
  FD_SET(pipeFd[0], &set);
  fd_max = fd_skt;
  
  while(1)
  {
    temp_set = set;
    if (select(fd_max+1, &tmpset, NULL, NULL, NULL) == -1) 
    {
	    perror("Error in select");
	    exit(EXIT_FAILURE);
	  }
	  for(int i=0; i <= fd_max; i++) 
	  {
	    if (FD_ISSET(i, &tmpset)) 
	    {
		    if (i == fd_skt) 
		    {
		      if ((fd = accept(fd_skt, (struct sockaddr*)NULL ,NULL)) == -1) 
		      {
			      perror("Error in accept");
			      exit(EXIT_FAILURE);
		      }
	      }
	      else if(i = pipeFd[0])
	      {
	        if(read(pipeFd[0], &fd, sizeof(int)) != sizeof(int))
	        {
	          perror("Error in pipe reading");
	          exit(EXIT_FAILURE);
	        }
	        FD_SET(fd, &set);
	        continue;
	      }
	      else
	      {
	        fd = i;
	      }
	      
	      FD_CLR(fd, &set);
	         
	      MALLOC(fdPtr, sizeof(int))
	      *fdPtr = fd;
	      S_enqueue(&(param.worker_queue), fdPtr);
	      
	    }
	  }
	}
	
	return 0;
}
