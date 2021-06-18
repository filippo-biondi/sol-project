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
  int byte_read;
  int client_disc;
  
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
      

      if((byte_read = read(request->fd, recived, sizeof(struct firstmessage))) != sizeof(struct firstmessage))
      {
        if(byte_read == 0)
        {
          closeOpenedFile(files, request->fd);
          close(request->fd);
          negfd = -(request->fd);
          if(write(((struct thread_args*) arg)->pipe_fd, &negfd, sizeof(int)) != sizeof(int))
          {
            perror("Pipe error:");
            exit(EXIT_FAILURE);
          }
          free(request);
          continue;
        }
        printf("Invalid firstmessage\n");
        response.op = 'b';
        SEND_FIRST_MESSAGE(response)
        free(request);
        continue;
      }
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
          if(errno == EPERM)
          {
            response.op = 'n';
            SEND_FIRST_MESSAGE(response)
            free(path);
            break;
          }
          else
          {
            response.op = 'n';
            SEND_FIRST_MESSAGE(response)
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
            if(write(request->fd, buf, size) != size)
            {
              perror("Writing error");  
              if(errno == EPIPE)
              {
                client_disc = 1;
              }
              else
              {                                                
                exit(EXIT_FAILURE);
              }
            }
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
        
        if(writeFile(files, request->fd, path, NULL) == -1)
        {
          if(errno == EPERM)
          {
            response.op = 'p';
          }
          if(errno == EFBIG)
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
        break;
        
      case 'a':
        
        if(request->t == 'n')
        {
          MALLOC(path, recived->size1)
          READ_PATH(path, recived->size1)
          MALLOC(buf, recived->size2)
          if(read(request->fd, buf, recived->size2) != recived->size2)
          {
            printf("Invalid message\n");
            response.op = 'b';
            SEND_FIRST_MESSAGE(response)
            free(path);
            free(buf);
            break;
          }
        }
        else
        {
          path = request->path;
          buf = request->buf;
          recived->size2 = request->buf_len;
        }
        if(appendFile(files, request->fd, path, buf, recived->size2, NULL) == -1)
        {
          if(errno = EACCES)
          {
            free(request);
            break;
          }
          if(errno = EPERM)
          {
            response.op = 'p';
          }
          if(errno = EFBIG)
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
          if(errno == EPERM)
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
      closeOpenedFile(files, request->fd);
      negfd = -(request->fd);
      if(request->path != NULL)
      {
        free(path);
      }
      if(request->buf != NULL)
      {
        free(buf);
      }
      free(request);
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
