#include <server_op.h>

SharedQueue work_queue;
int pipe_fd;

long int max_storage = 100000000;
int max_n_file = 100;

int fd_max;

extern void* worker_routine(void* arg);

int main(int argc, char* argv[])
{
  int n_worker = 10;
  
  struct storage files;
  char* socket_name;
  int fd_skt;
  struct sockaddr_un sa;
  fd_set set;
  fd_set tmpset;
  int fd;
  int* fdPtr;
  int pipeFd[2];
  FILE* config;
  int n_buckets;
  
  if(argc > 2)
  {
    printf("Invalid arguments\n");
    return 0;
  }
  
  if(argc == 2)
  {
    if((config = fopen(argv[1], "r")) == NULL)
    {
      perror("Error in config file opening, default values used");
    }
    else
    {
      if(fscanf(config, "N_WORKER = %d", &n_worker) != 1)
      {
        perror("Error in config file reading, default values for N_WORKER used");
      }
      fscanf(config, "%*c");
      if(fscanf(config, "MAX_STORAGE = %ld", &max_storage) != 1)
      {
        perror("Error in config file reading, default values for MAX_STORAGE used");
      }
      fscanf(config, "%*c");
      if(fscanf(config, "MAX_N_FILE = %d", &max_n_file) != 1)
      {
        perror("Error in config file reading, default values for MAX_N_FILE used");
      }
      fscanf(config, "%*c");
      if(fscanf(config, "N_BUCKETS = %d", &n_buckets) != 1)
      {
        n_buckets = 2 * max_n_file;
        perror("Error in config file reading, default values for N_BUCKETS used");
      }
      fscanf(config, "%*c");
      MALLOC(socket_name, NAME_MAX+1)
      if(fscanf(config, "SOCKET_NAME = %s", socket_name) != 1)
      {
        strcpy(socket_name, "socket");
        perror("Error in config file reading, default values for SOCKET NAME used");
      }
      fscanf(config, "%*c");
    }
  }
  
  initStorage(&files, n_buckets);
  
  initQueue(&work_queue);
  
  if(pipe(pipeFd) == -1)
  {
    perror("Error in pipe creating");
    exit(EXIT_FAILURE);
  }
  pipe_fd = pipeFd[1];
  
  pthread_t workers[n_worker];
  for(int i=0; i < n_worker; i++)
  {
    if(pthread_create(&(workers[i]), NULL, worker_routine, &files) != 0)
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
  
  strncpy(sa.sun_path, socket_name, NAME_MAX);  
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
  FD_ZERO(&tmpset);
  FD_SET(fd_skt, &set);
  FD_SET(pipeFd[0], &set);
  fd_max = fd_skt;
  
  while(1)
  {
    tmpset = set;
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
	      else if(i == pipeFd[0])
	      {
	        if(read(pipeFd[0], &fd, sizeof(int)) != sizeof(int))
	        {
	          perror("Error in pipe reading");
	          exit(EXIT_FAILURE);
	        }
	        if(fd > fd_max)
	        {
	          fd_max = fd;
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
	      if(S_enqueue(&work_queue, fdPtr))
	      {
	        perror("Error in worker queue");
          exit(EXIT_FAILURE);
	      }
	    }
	  }
	}
	
	return 0;
}
