#include <server_op.h>

void* worker_routine(void* arg)
{
  int* fd;
  int negfd;
  struct firstmessage* recived;
  struct firstmessage response;
  struct storage* files = ((struct thread_args*) arg)->files;
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
    if(S_dequeue(((struct thread_args*) arg)->work_queue, (void**) &fd) == -1)
    {
      perror("Error in worker queue");
      exit(EXIT_FAILURE);
    }
    
    if(*fd == -1)
    {
      if(S_insert_tail(((struct thread_args*) arg)->work_queue, fd))
	    {
	      perror("Error in worker queue");
	    }
	    break;
    }
    
    
    if((byte_read = read(*fd, recived, sizeof(struct firstmessage))) != sizeof(struct firstmessage))
    {
      if(byte_read == 0)
      {
        closeOpenedFile(files, *fd);
        close(*fd);
        negfd = -(*fd);
        free(fd);
        if(write(((struct thread_args*) arg)->pipe_fd, &negfd, sizeof(int)) != sizeof(int))
        {
          perror("Pipe error:");
          exit(EXIT_FAILURE);
        }
        continue;
      }
      printf("Invalid firstmessage\n");
      response.op = 'b';
      SEND_FIRST_MESSAGE(response)
      free(fd);
      continue;
    }
    
    client_disc = 0;
    
    switch(recived->op)
    {
      case 'o':
        MALLOC(path, recived->size1)
        READ_PATH(path, recived->size1)
        if(read(*fd, &flags, recived->size2) != recived->size2)
        {
          printf("Invalid message\n");
          response.op = 'b';
          SEND_FIRST_MESSAGE(response)
          free(path);
          continue;
        }
        if(openFile(files, *fd, path, flags) == -1)
        {
          if(errno == EEXIST)
          {
            response.op = 'e';
          }
          if(errno == ENOENT)
          {
            response.op = 'n';
          }
        }
        else
        {
          response.op = 'y';
        }
        SEND_FIRST_MESSAGE(response)
        break;
        
      case 'r':
        MALLOC(path, recived->size1)
        READ_PATH(path, recived->size1)
        if(readFile(files, *fd, path, &buf, &size) == -1)
        {
          response.op = 'n';
          SEND_FIRST_MESSAGE(response)
          free(path);
          continue;
        }
        else
        {
          response.op = 'y';
          response.size1 = size;
          SEND_FIRST_MESSAGE(response)
          if(client_disc == 0)
          {
            if(write(*fd, buf, size) != size)
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
        break;
        
      case 'R':
        if(read(*fd, &N, recived->size1) != recived->size1)
        {
          printf("Invalid message\n");
          response.op = 'b';
          SEND_FIRST_MESSAGE(response)
          continue;
        }
        
        if(readNFile(files, *fd, N) == -1)
        {
          if(errno == EPIPE)
          {
            client_disc = 1;
          }
          perror("error in file reading");
        }

        break;
        
      case 'w':
        MALLOC(path, recived->size1)
        READ_PATH(path, recived->size1)
        
        if(writeFile(files, *fd, path, NULL) == -1)
        {
          response.op = 'n';
        }
        else
        {
          response.op = 'y';
        }
        
        SEND_FIRST_MESSAGE(response)
        free(path);
        break;
        
      case 'a':
        MALLOC(path, recived->size1)
        READ_PATH(path, recived->size1)
        MALLOC(buf, recived->size2)
        if(read(*fd, buf, recived->size2) != recived->size2)
        {
          printf("Invalid message\n");
          response.op = 'b';
          SEND_FIRST_MESSAGE(response)
          free(path);
          free(buf);
          continue;
        }
        
        if(appendFile(files, *fd, path, buf, recived->size2, NULL) == -1)
        {
          response.op = 'n';
        }
        else
        {
          response.op = 'y';
        }
        SEND_FIRST_MESSAGE(response)
        free(path);
        free(buf);
        break;
      
      case 'l':
        MALLOC(path, recived->size1)
        READ_PATH(path, recived->size1)
        if(lock(files, *fd, path) == -1)
        {
          response.op = 'n';
        }
        else
        {
          response.op = 'y';
        }
        SEND_FIRST_MESSAGE(response)
        free(path);
        break;
        
      case 'u':
        MALLOC(path, recived->size1)
        READ_PATH(path, recived->size1)
        if(unlock(files, *fd, path) == -1)
        {
          response.op = 'n';
        }
        else
        {
          response.op = 'y';
        }
        SEND_FIRST_MESSAGE(response)
        free(path);
        break;
        
      case 'c':
        MALLOC(path, recived->size1)
        READ_PATH(path, recived->size1)
        if(closeFile(files, *fd, path, *(((struct thread_args*) arg)->fd_max)) == -1)
        {
          response.op = 'n';
        }
        else
        {
          response.op = 'y';
        }
        SEND_FIRST_MESSAGE(response)
        free(path);
        break;
        
      case 'd':
        MALLOC(path, recived->size1)
        READ_PATH(path, recived->size1)
        if(deleteFile(files, *fd, path, *(((struct thread_args*) arg)->fd_max)) == -1)
        {
          response.op = 'n';
        }
        else
        {
          response.op = 'y';
        }
        SEND_FIRST_MESSAGE(response)
        free(path);
        break;
      default:
        printf("Invalid request\n");
        response.op = 'b';
        SEND_FIRST_MESSAGE(response)
        continue;
    }
    
    if(client_disc == 1)
    {
      closeOpenedFile(files, *fd);
      negfd = -(*fd);
      free(fd);
      if(write(((struct thread_args*) arg)->pipe_fd, &negfd, sizeof(int)) != sizeof(int))
      {
        perror("Pipe error:");
        exit(EXIT_FAILURE);
      }
      continue;
    }
    
    if(write(((struct thread_args*) arg)->pipe_fd, fd, sizeof(int)) != sizeof(int))
    {
      perror("Pipe error:");
      exit(EXIT_FAILURE);
    }
    free(fd);
  }
  free(recived);
  return NULL;
}
