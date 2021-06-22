#include <server_op.h>

extern void* worker_routine(void* arg);
extern void* signal_handler(void* arg);

int main(int argc, char* argv[])
{
  int n_worker = DEFAULT_N_WORKER;
  
  struct thread_args args;;
  struct storage files;
  char* socket_name;
  int fd_skt;
  struct sockaddr_un sa;
  fd_set set;
  fd_set tmpset;
  fd_set connected;
  int fd;
  int fd_max;
  struct request* new_request;
  int pipeFd[2];
  int handler_pipe[2];
  int term = 0;
  FILE* config;
  int n_buckets;
  int k;
  SharedQueue work_queue;
  sigset_t mask;
  struct handler_args argh;
  pthread_t sighandler_thread;
  
  sigemptyset(&mask);
  sigaddset(&mask, SIGINT); 
  sigaddset(&mask, SIGQUIT);
  sigaddset(&mask, SIGHUP);
  sigaddset(&mask, SIGPIPE);
  
  if ((errno = pthread_sigmask(SIG_BLOCK, &mask, NULL)) != 0) //mask the 3 signal to handle and SIGPIPE in order to bee interrupted if client disconnect suddently while sending a response
  {
	  perror("Error in masking signal");
	  exit(EXIT_FAILURE);
  }
  
  if(pipe(handler_pipe) == -1)  //pipe used by signal handler for communication with main
  {
    perror("Error in pipe creating");
    exit(EXIT_FAILURE);
  }
  
  argh.pipe_fd = handler_pipe[1];
  argh.terminate = &term;
  
  if(pthread_create(&sighandler_thread, NULL, signal_handler, &argh) != 0)  //creation of signal handler
  {
    perror("Error in signal handler thread creation");
    exit(EXIT_FAILURE);
  }
  
  
  if(argc > 2)
  {
    printf("Invalid arguments\n");
    return 0;
  }
  
  if(argc == 2 && (config = fopen(argv[1], "r")) != NULL)  //attempt to read config file
  {
    if(fscanf(config, "N_WORKER = %d", &n_worker) != 1)
    {
      printf("Error in config file reading, default values for N_WORKER used\n");
    }
    fscanf(config, "%*c");
    if(fscanf(config, "MAX_STORAGE = %ld", &(files.max_storage)) != 1)
    {
      printf("Error in config file reading, default values for MAX_STORAGE used\n");
      files.max_storage = DEFAULT_MAX_STORAGE;
    }
    fscanf(config, "%*c");
    if(fscanf(config, "MAX_N_FILE = %d", &(files.max_n_file)) != 1)
    {
      printf("Error in config file reading, default values for MAX_N_FILE used\n");
      files.max_n_file = DEFAULT_MAX_N_FILE;
    }
    fscanf(config, "%*c");
    if(fscanf(config, "N_BUCKETS = %d", &n_buckets) != 1)
    {
      n_buckets = 1.33 * files.max_n_file;
      printf("Error in config file reading, default values for N_BUCKETS used\n");
    }
    fscanf(config, "%*c");
    MALLOC(socket_name, NAME_MAX+1)
    if(fscanf(config, "SOCKET_NAME = %s", socket_name) != 1)
    {
      strcpy(socket_name, DEFAULT_SOCKET);
      printf("Error in config file reading, default values for SOCKET NAME used\n");
    }
    fclose(config);
  }
  else  //if fail default values are used
  {
    printf("Config file not found, default values used\n");
    files.max_storage = DEFAULT_MAX_STORAGE;
    files.max_n_file = DEFAULT_MAX_N_FILE;
    n_buckets = 2 * files.max_n_file;
    MALLOC(socket_name, NAME_MAX+1)
    strcpy(socket_name, DEFAULT_SOCKET);
  }
  
  strncpy(sa.sun_path, socket_name, NAME_MAX);  
  free(socket_name);
  sa.sun_family = AF_UNIX;
  if((fd_skt = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
  {
    perror("Error in socket creating");
    pthread_kill(sighandler_thread, 1);
    pthread_join(sighandler_thread, NULL);
    exit(EXIT_FAILURE);
  }
  
  if(bind(fd_skt, (struct sockaddr *)&sa, sizeof(sa)) == -1)
  {
    perror("Error in socket binding");
    pthread_kill(sighandler_thread, 1);
    pthread_join(sighandler_thread, NULL);
    exit(EXIT_FAILURE);
  }
  if(listen(fd_skt, SOMAXCONN) == -1)
  {
    perror("Error in socket listening");
    pthread_kill(sighandler_thread, 1);
    pthread_join(sighandler_thread, NULL);
    unlink(sa.sun_path);
    exit(EXIT_FAILURE);
  }
  
  if(initStorage(&files, n_buckets) == -1)
  {
    perror("Error in hash creating");
    pthread_kill(sighandler_thread, 1);
    pthread_join(sighandler_thread, NULL);
    unlink(sa.sun_path);
    exit(EXIT_FAILURE);
  }
  
  if(pipe(pipeFd) == -1)  //pipe use by worker for communicate with main
  {
    perror("Error in pipe creating");
    pthread_kill(sighandler_thread, 1);
    pthread_join(sighandler_thread, NULL);
    unlink(sa.sun_path);
    exit(EXIT_FAILURE);
  }
  
  initQueue(&work_queue);

  args.pipe_fd = pipeFd[1];
  args.files = &files;
  args.work_queue = &work_queue;
  args.fd_max = &fd_max;
  
  pthread_t workers[n_worker];
  for(int i=0; i < n_worker; i++)  //spawn pool of worker
  {
    if(pthread_create(&(workers[i]), NULL, worker_routine, &args) != 0)
    {
      perror("Error in creating pool thread");
      if(i == 0)
      {
        pthread_kill(sighandler_thread, 1);
        pthread_join(sighandler_thread, NULL);
        unlink(sa.sun_path);
        icl_hash_destroy(files.hashT, free_key, free_data);
        exit(EXIT_FAILURE);
      }
      else  //if error appen but at least one worker has spawned server can continue 
      {
        printf("Only %d thread in the pool\n", i);
        n_worker = i;
        break;
      }
    }
  }
  
  FD_ZERO(&set);
  FD_ZERO(&tmpset);
  FD_ZERO(&connected);  //set to keep trak of still connected client
  FD_SET(fd_skt, &set);  //listen on socket
  FD_SET(pipeFd[0], &set); //listen on worker pipe 
  FD_SET(handler_pipe[0], &set); //listen on signal handler pipe
  fd_max = fd_skt;
  if(pipeFd[0] > fd_max)
  {
    fd_max = pipeFd[0];
  }
  
  if(handler_pipe[0] > fd_max)
  {
    fd_max = handler_pipe[0];
  }
  
  while(1)
  {
    if(term == -1)  //if SIGINT or SIGQUIT arrived exit immediatly
    {
      break;
    }
    if(term == 1)  //if SIGHUP arrived check if there are still open connection and exit if there aren't
    {
      for(k = 0; k <= fd_max; k++)
      {
        if(FD_ISSET(k, &connected))
        {
          break;
        }
      } 
      if(k == fd_max + 1)
      {
        break;
      }
    }
    tmpset = set;
    if(select(fd_max + 1, &tmpset, NULL, NULL, NULL) == -1)
    {
	    perror("Error in select");
	    pthread_kill(sighandler_thread, 2);
	    break;
	  }
	  for(int i=0; i <= fd_max; i++)  //understand whitch file descriptor is avaible for reading
	  {
	    if (FD_ISSET(i, &tmpset)) 
	    {
		    if(i == fd_skt)  //new connection
        {
          if ((fd = accept(fd_skt, (struct sockaddr*)NULL ,NULL)) == -1) 
		      {
		        perror("Error in accept");
			      pthread_kill(sighandler_thread, 2);
		      }
		      if(fd > fd_max)
	        {
	          fd_max = fd;
	        }
	        FD_SET(fd, &connected);
	      }
	      else if(i == pipeFd[0])  //worker pipe
	      {
	        if(read(pipeFd[0], &fd, sizeof(int)) != sizeof(int))
	        {
	          perror("Error in pipe reading");
	          pthread_kill(sighandler_thread, 2);
	        }
	        if(fd < 0)  //a client has disconnected (whe a client disconnect his file descriptor with changed sign is written on pipe
	        {
	          FD_CLR(-fd, &connected);  //remove its file descriptor from connected set
	        }
	        else  //a client request has been executed and client is still connected
	        {
	          FD_SET(fd, &set);  //we listen for new request from that client
	        }
	        continue;
	      }
	      else if(i == handler_pipe[0])  //a signal has arrived and handler thread has closed its pipe
        {
	        FD_CLR(fd_skt, &set);  //don't listen for new connection anymore
	        FD_CLR(handler_pipe[0], &set);
	        pthread_join(sighandler_thread, NULL);
	        continue;
	      }  
	      else  //the request is from a client that was already connected
	      {
	        fd = i;
        }
	      
	      FD_CLR(fd, &set);  //don't listen anymore for request from client untill worker will notyfy that the request has been executed
	         
	      if((new_request = malloc(sizeof(struct request))) == NULL)  //create a new request
	      {
	        perror("Error in malloc");
          pthread_kill(sighandler_thread, 2);
          break;
	      }  
	      new_request->t = 'n';  //indicate a new request (and not a pending request)
	      new_request->fd = fd;
	      new_request->path = NULL;
	      new_request->buf = NULL;
	      new_request->buf_len = 0;
	      new_request->flags = 0;
	      if(S_enqueue(&work_queue, new_request) != 0)
	      {
	        perror("Error in worker queue");
          pthread_kill(sighandler_thread, 2);
          break;
	      }
	    }
	  }
	}
	pthread_join(sighandler_thread, NULL);

	if((new_request = malloc(sizeof(struct request))) == NULL)  //request for signaling to worker that it's time to exit
	{
	  perror("Error in malloc");
	}
	else
	{
	  new_request->t = 'n';
	  new_request->fd = -1;
	  new_request->path = NULL;
	  new_request->buf = NULL;
	  new_request->buf_len = 0;
	  
	  if(S_insert_tail(&work_queue, new_request) != 0)
	  {
	    perror("Error in worker queue");
    }
    else
    {	
	    for(int i=0; i < n_worker; i++)
	    {
	      if(pthread_join(workers[i], NULL) != 0)
	      {
	        perror("Error in thread join");
	      }
	    }
	  }
	}
	
	close(fd_skt);
	close(pipeFd[0]);
	close(pipeFd[1]);
	close(handler_pipe[0]);
	unlink(sa.sun_path);
	
	printf("Max number of file memorized in the server: %ld\n", files.max_reached_n_file);
	printf("Max dimension reached by the server: %fMB\n", (float) files.max_reached_storage / 1000000);
	printf("There have been %d replacement for a total of %d victim file selected\n", files.n_replacement,files.n_victim);
  
  printf("%d ", files.n_saved_file);
  my_icl_hash_dump(stdout, files.hashT);
  
	icl_hash_destroy(files.hashT, free_key, free_data);
	 
  while(work_queue.tail != NULL)
 	{
	  if(S_dequeue(&work_queue, (void**) &new_request) == 0)
	  {
	    free(new_request);
	  }
	}
	
	return 0;
}
