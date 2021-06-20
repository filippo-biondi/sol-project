#include <server_op.h>

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
  
  memset(&response, 0, sizeof(struct firstmessage));
  MALLOC(recived, sizeof(struct firstmessage))
  
  while(1)
  {
    if(S_dequeue(((struct thread_args*) arg)->work_queue, (void**) &request) == -1)
    {
      perror("Error in worker queue");
      exit(EXIT_FAILURE);
    }
    
    fd = request->fd;
    
    if(request->t != 'n')
    {
      recived->op = request->t;
      recived->size1 = strlen(request->path) + 1;
    }
    else
    {
      if(request->fd == -1)
      {
        if(S_insert_tail(((struct thread_args*) arg)->work_queue, request))
	      {
	        perror("Error in worker queue");
	      }
	      break;
      }
      
      READ_FIRST_MESSAGE(recived)
      
    }
    client_disc = 0;
    
    switch(recived->op)
    {
      case 'o':
        if(request->t == 'n')
        {
          MALLOC(path, recived->size1)
          READ_PATH(path, recived->size1)
          if(read(request->fd, &flags, recived->size2) != recived->size2)
          {
            printf("Invalid message\n");
            response.op = 'b';
            SEND_FIRST_MESSAGE(response)
            free(path);
            break;
          }
        }
        else
        {
          path = request->path;
          flags = request->flags;
        }
      
        if(openFile(files, request->fd, path, flags) == -1)
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
          if(errno == EACCES)
          {
            free(request);
            break;
          }
        }
        else
        {
          response.op = 'y';
        }
        SEND_FIRST_MESSAGE(response)
        free(request);
        free(path);
        break;
        
      case 'r':
        
        if(request->t == 'n')
        {
          MALLOC(path, recived->size1)
          READ_PATH(path, recived->size1)
        }
        else
        {
          path = request->path;
        }
        if(readFile(files, request->fd, path, &buf, &size) == -1)
        {
          if(errno == EACCES)
          {
            free(request);
            break;
          }
          else if(errno == EPERM)
          {
            response.op = 'p';
            SEND_FIRST_MESSAGE(response)
            free(request);
            free(path);
            break;
          }
          else
          {
            response.op = 'n';
            SEND_FIRST_MESSAGE(response)
            free(request);
            free(path);
            break;
          }
        }
        else
        {
          response.op = 'y';
          response.size1 = size;
          SEND_FIRST_MESSAGE(response)
          if(client_disc == 0)
          {
            errno = 0;
            while((written_bytes += write(request->fd, buf + written_bytes, size - written_bytes)) != size)
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
            exit(EXIT_FAILURE);
          }
        }
        free(path);
        free(buf);
        free(request);
        break;
        
      case 'R':
        if(read(request->fd, &N, recived->size1) != recived->size1)
        {
          printf("Invalid message\n");
          response.op = 'b';
          SEND_FIRST_MESSAGE(response)
          break;
        }
        
        if(readNFile(files, request->fd, N) == -1)
        {
          if(errno == EPIPE)
          {
            client_disc = 1;
          }
          perror("error in file reading");
        }
        free(request);
        break;
        
      case 'w':
        MALLOC(path, recived->size1)
        READ_PATH(path, recived->size1)
        MALLOC(buf, recived->size2)
        READ_BUF(buf, recived->size2)
        if(writeFile(files, request->fd, path, buf, recived->size2, NULL) == -1)
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
        
        SEND_FIRST_MESSAGE(response)
        free(request);
        free(path);
        free(buf);
        break;
        
      case 'a':
        
        if(request->t == 'n')
        {
          MALLOC(path, recived->size1)
          READ_PATH(path, recived->size1)
          MALLOC(buf, recived->size2)
          READ_BUF(buf, recived->size2)
        }
        else
        {
          path = request->path;
          buf = request->buf;
          recived->size2 = request->buf_len;
        }
        if(appendFile(files, request->fd, path, buf, recived->size2, NULL) == -1)
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
        SEND_FIRST_MESSAGE(response)
        free(request);
        free(path);
        free(buf);
        break;
      
      case 'l':
        MALLOC(path, recived->size1)
        if(request->t == 'n')
        {
          READ_PATH(path, recived->size1)
        }
        else
        {
          strcpy(path, request->path);
        }
        if(lock(files, request->fd, path) == -1)
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
        SEND_FIRST_MESSAGE(response)
        free(request);
        free(path);
        break;
        
      case 'u':
        MALLOC(path, recived->size1)
        READ_PATH(path, recived->size1)
        if(unlock(arg, request->fd, path) == -1)
        {
          response.op = 'n';
        }
        else
        {
          response.op = 'y';
        }
        SEND_FIRST_MESSAGE(response)
        free(request);
        free(path);
        break;
        
      case 'c':
        MALLOC(path, recived->size1)
        READ_PATH(path, recived->size1)
        if(closeFile(arg, request->fd, path, *(((struct thread_args*) arg)->fd_max)) == -1)
        {
          response.op = 'n';
        }
        else
        {
          response.op = 'y';
        }
        SEND_FIRST_MESSAGE(response)
        free(request);
        free(path);
        break;
        
      case 'd':
        MALLOC(path, recived->size1)
        READ_PATH(path, recived->size1)
        if(deleteFile(files, request->fd, path, *(((struct thread_args*) arg)->fd_max)) == -1)
        {
          response.op = 'n';
        }
        else
        {
          response.op = 'y';
        }
        SEND_FIRST_MESSAGE(response)
        free(request);
        free(path);
        break;
      default:
        printf("Invalid request\n");
        response.op = 'b';
        SEND_FIRST_MESSAGE(response)
        free(request);
        break;
    }
    
    if(client_disc == 1)
    {
      closeOpenedFile(files, fd, *(((struct thread_args*) arg)->fd_max));
      negfd = -(fd);
      //free(request);
      if(write(((struct thread_args*) arg)->pipe_fd, &negfd, sizeof(int)) != sizeof(int))
      {
        perror("Pipe error:");
        exit(EXIT_FAILURE);
      }
      continue;
    }
    
    if(write(((struct thread_args*) arg)->pipe_fd, &fd, sizeof(int)) != sizeof(int))
    {
      perror("Pipe error:");
      exit(EXIT_FAILURE);
    }
  }
  free(recived);
  return NULL;
}
