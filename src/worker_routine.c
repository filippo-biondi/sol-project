#include <server_op.h>


void free_recived(void* recived)
{
  free(recived);
}

void* worker_routine(void* arg)
{
  int negfd;
  int fd;
  struct firstmessage* recived;
  struct firstmessage response;
  struct storage* files = ((struct thread_args*) arg)->files;
  struct request* request;
  char* path;
  int flags;
  void* buf;
  size_t size;
  int N;
  long int byte_read = 0;
  int client_disc;
  long int written_bytes = 0;
  int wr_error = 0;
  
  memset(&response, 0, sizeof(struct firstmessage));
  WORKER_MALLOC(recived, sizeof(struct firstmessage))
  
  pthread_cleanup_push(free_recived, recived);
  
  while(1)
  {
    if(S_dequeue(((struct thread_args*) arg)->work_queue, (void**) &request) == -1)  //try to read a request, if there aren't thread wait on a condition variable (see in shared_queue.c)
    {
      perror("Error in worker queue");
      pthread_exit(NULL);
    }
    
    fd = request->fd;
    
    if(request->t != 'n')  //if is a pending request
    {
      recived->op = request->t;
      recived->size1 = strlen(request->path) + 1;
    }
    else  //if is a new request
    {
      if(fd == -1)  //if is a termination signal put back the request in the queue and exit from the loop
      {
        if(S_insert_tail(((struct thread_args*) arg)->work_queue, request))
	      {
	        perror("Error in worker queue");
	      }
	      break;
      }
      
      READ_FIRST_MESSAGE(recived)  //else read the message from client
      
    }
    client_disc = 0;
    
    switch(recived->op)
    {
      case 'o':
        if(request->t == 'n')  //if is a new request read path and flags from client
        {
          WORKER_MALLOC(path, recived->size1)
          READ_PATH(path, recived->size1)
          if(read(fd, &flags, recived->size2) != recived->size2)
          {
            printf("Invalid message\n");
            response.op = 'b';
            free(path);
            SEND_FIRST_MESSAGE(response)
            break;
          }
        }
        else  //else take them from the pending request
        {
          path = request->path;
          flags = request->flags;
        }
      
        if(openFile(files, fd, path, flags) == -1)
        {
          if(errno == EEXIST)
          {
            response.op = 'e';
          }
          if(errno == ENOENT)
          {
            response.op = 'n';
          }
          if(errno == EPERM)
          {
            response.op = 'p';
          }
          if(errno == EACCES)  //can't access the file because lock is holded by another client, the request has already been enqueued in file's waiting queue
          {
            free(request);
            break;
          }
        }
        else
        {
          response.op = 'y';
        }
        free(request);
        free(path);
        SEND_FIRST_MESSAGE(response)
        break;
        
      case 'r':
        
        if(request->t == 'n')
        {
          WORKER_MALLOC(path, recived->size1)
          READ_PATH(path, recived->size1)
        }
        else
        {
          path = request->path;
        }
        if(readFile(files, fd, path, &buf, &size) == -1)
        {
          if(errno == EACCES)
          {
            free(request);
            break;
          }
          else if(errno == EPERM)
          {
            response.op = 'p';
            free(request);
            free(path);
            SEND_FIRST_MESSAGE(response)
            break;
          }
          else
          {
            response.op = 'n';
            free(request);
            free(path);
            SEND_FIRST_MESSAGE(response)
            break;
          }
        }
        else
        {
          response.op = 'y';
          response.size1 = size;
          errno = 0;
          SEND_FIRST_MESSAGE(response)
          if(client_disc == 0 && errno == 0)  //if client is still connected (client_disc could be updated in SEND_FIRST_MESSAGE if errno == EPIPE) and no other errors on write have occured 
          {
            while((written_bytes += write(fd, buf + written_bytes, size - written_bytes)) != size)
            {
              if(errno != 0)
              {
                break;
              }
            }
            written_bytes = 0;
          }
 
          if(errno == EPIPE)
          {
            client_disc = 1;
          }
          else if(errno != 0)
          {  
            perror("Writing error");                                          
            wr_error = 1;  //this cause the connection to the client to be interrupted (see after the end of switch)
          }
        }
        free(path);
        free(buf);
        free(request);
        break;
        
      case 'R':
        if(read(fd, &N, recived->size1) != recived->size1)
        {
          printf("Invalid message\n");
          response.op = 'b';
          SEND_FIRST_MESSAGE(response)
          break;
        }
        
        if(readNFile(files, fd, N) == -1)
        {
          if(errno == EPIPE)
          {
            client_disc = 1;
          }
          else
          {
            perror("error in file reading");
            wr_error = 1;
          }
        } 
        free(request);
        break;
        
      case 'w':
        WORKER_MALLOC(path, recived->size1)
        READ_PATH(path, recived->size1)
        MALLOC_BUF(buf, recived->size2)
        READ_BUF(buf, recived->size2)
        if(writeFile(arg, fd, path, buf, recived->size2, NULL) == -1)
        {
          if(errno == EPERM)
          {
            response.op = 'p';
          }
          else if(errno == EFBIG)
          {
            response.op = 't';
          }
          else
          {
            response.op = 'n';
          }
        }
        else
        {
          response.op = 'y';
        }
        free(request);
        free(path);
        free(buf);
        SEND_FIRST_MESSAGE(response)        
        break;
        
      case 'a':
        
        if(request->t == 'n')
        {
          WORKER_MALLOC(path, recived->size1)
          READ_PATH(path, recived->size1)
          MALLOC_BUF(buf, recived->size2)
          READ_BUF(buf, recived->size2)
        }
        else
        {
          path = request->path;
          buf = request->buf;
          recived->size2 = request->buf_len;
        }
        if(appendFile(arg, fd, path, buf, recived->size2, NULL) == -1)
        {
          if(errno == EACCES)
          {
            free(request);
            break;
          }
          else if(errno == EPERM)
          {
            response.op = 'p';
          }
          else if(errno == EFBIG)
          {
            response.op = 't';
          }
          else
          {
            response.op = 'n';
          }
        }
        else
        {
          response.op = 'y';
        }
        free(request);
        free(path);
        free(buf);
        SEND_FIRST_MESSAGE(response)
        break;
      
      case 'l':
        
        if(request->t == 'n')
        {
          WORKER_MALLOC(path, recived->size1)
          READ_PATH(path, recived->size1)
        }
        else
        {
          path = request->path;
        }
        if(lock(files, fd, path) == -1)
        {
          if(errno == EACCES)
          {
            free(request);
            break;
          }
          else if(errno == EPERM)
          {
            response.op = 'p';
          }
          else
          {
            response.op = 'n';
          }
          
        }
        else
        {
          response.op = 'y';
        }
        free(request);
        free(path);
        SEND_FIRST_MESSAGE(response)
        break;
        
      case 'u':
        WORKER_MALLOC(path, recived->size1)
        READ_PATH(path, recived->size1)
        if(unlock(arg, fd, path) == -1)
        {
          response.op = 'n';
        }
        else
        {
          response.op = 'y';
        }
        free(request);
        free(path);
        SEND_FIRST_MESSAGE(response)
        break;
        
      case 'c':
        WORKER_MALLOC(path, recived->size1)
        READ_PATH(path, recived->size1)
        if(closeFile(arg, fd, path, *(((struct thread_args*) arg)->fd_max)) == -1)
        {
          response.op = 'n';
        }
        else
        {
          response.op = 'y';
        }
        free(request);
        free(path);
        SEND_FIRST_MESSAGE(response)
        break;
        
      case 'd':
        WORKER_MALLOC(path, recived->size1)
        READ_PATH(path, recived->size1)
        if(deleteFile(files, fd, path, *(((struct thread_args*) arg)->fd_max)) == -1)
        {
          response.op = 'n';
        }
        else
        {
          response.op = 'y';
        }
        free(request);
        free(path);
        SEND_FIRST_MESSAGE(response)
        break;
      default:
        printf("Invalid request\n");
        response.op = 'b';
        free(request);
        SEND_FIRST_MESSAGE(response)
        break;
    }
    
    if(client_disc == 1 || wr_error == 1)  //if client disconnected or an error on write has occured inform main thread that client is disconnected
    {
      closeOpenedFile(files, fd, *(((struct thread_args*) arg)->fd_max));
      negfd = -(fd);
      if(write(((struct thread_args*) arg)->pipe_fd, &negfd, sizeof(int)) != sizeof(int))
      {
        perror("Pipe error:");
        pthread_exit(NULL);
      }
      wr_error = 0;
      continue;
    }
    
    if(write(((struct thread_args*) arg)->pipe_fd, &fd, sizeof(int)) != sizeof(int))  //else inform that client request has been executed and is possible to accept other request 
    {
      perror("Pipe error:");
      pthread_exit(NULL);
    }
  }
  pthread_cleanup_pop(1);
  return NULL;
}
