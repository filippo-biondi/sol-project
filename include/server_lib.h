#ifndef _SERVER_LIB
#define _SERVER_LIB

#include <utils.h>
#include <pthread.h>
#include <sys/select.h>
#include <shared_queue.h>
#include <icl_hash.h>
#include <signal.h>

#define O_CREATE 1
#define O_LOCK 2
#define DEFAULT_N_WORKER 10
#define DEFAULT_MAX_STORAGE 100000000
#define DEFAULT_MAX_N_FILE 1000
#define DEFAULT_SOCKET "socket"

#define WORKER_MALLOC(ptr, size) \
if((ptr = malloc(size)) == NULL) \
{                                \
  pthread_exit(NULL);            \
}                                \

#define MALLOC_BUF(ptr, size)    \
if((ptr = malloc(size)) == NULL) \
{                                \
  free(path);                    \
  pthread_exit(NULL);            \
}                                \

#define MALLOC_OP(ptr, size)     \
if((ptr = malloc(size)) == NULL) \
{                                \
  WRITER_UNLOCK                  \
  return -1;                     \
}                                \

#define MALLOC_OP_R(ptr, size)   \
if((ptr = malloc(size)) == NULL) \
{                                \
  READER_UNLOCK                  \
  return -1;                     \
}                                \

#define SEND_FIRST_MESSAGE(message)                                                  \
if(write(fd, &message, sizeof(struct firstmessage)) !=  sizeof(struct firstmessage)) \
{                                                                                    \
  if(errno == EPIPE)                                                                 \
  {                                                                                  \
    client_disc = 1;                                                                 \
  }                                                                                  \
  else                                                                               \
  {                                                                                  \
    wr_error = 1;                                                                    \
  }                                                                                  \
}                                                                                    \
    
#define READ_FIRST_MESSAGE(message)                                                             \
if((byte_read = read(fd, message, sizeof(struct firstmessage))) != sizeof(struct firstmessage)) \
{                                                                                               \
  if(byte_read == 0)                                                                            \
  {                                                                                             \
    closeOpenedFile(files, fd, *(((struct thread_args*) arg)->fd_max));                         \
    close(fd);                                                                                  \
    negfd = -(fd);                                                                              \
    free(request);                                                                              \
    if(write(((struct thread_args*) arg)->pipe_fd, &negfd, sizeof(int)) != sizeof(int))         \
    {                                                                                           \
      perror("Pipe error:");                                                                    \
      pthread_exit(NULL);                                                                       \
    }                                                                                           \
    continue;                                                                                   \
  }                                                                                             \
  printf("Invalid firstmessage\n");                                                             \
  response.op = 'b';                                                                            \
  free(request);                                                                                \
  SEND_FIRST_MESSAGE(response)                                                                  \
  continue;                                                                                     \
}                                                                                               \

#define READ_PATH(path, size)    \
if(read(fd, path, size) != size) \
{                                \
  printf("Invalid message\n");   \
  response.op = 'b';             \
  free(path);                    \
  SEND_FIRST_MESSAGE(response)   \
  break;                         \
}                                \

#define READ_BUF(buf, size)                                               \
byte_read = 0;                                                            \
errno = 0;                                                                \
while((byte_read += read(fd, buf + byte_read, size - byte_read)) != size) \
{                                                                         \
  if(errno != 0 || byte_read == 0)                                        \
  {                                                                       \
    break;                                                                \
  }                                                                       \
}                                                                         \
                                                                          \
if(errno != 0 || byte_read == 0)                                          \
{                                                                         \
  printf("Invalid message\n");                                            \
  response.op = 'b';                                                      \
  free(request);                                                          \
  free(path);                                                             \
  free(buf);                                                              \
  SEND_FIRST_MESSAGE(response)                                            \
  break;                                                                  \
}                                                                         \
byte_read = 0;                                                            \
  
struct saved_file
{
  char* name;
  void* buf;
  size_t size;
  int locked;
  int deleting;
  pthread_mutex_t mutex;
  struct timespec last_access;
  fd_set opened;
  SharedQueue* wait_queue;
};

struct storage
{
  icl_hash_t* hashT;
  long int max_storage;
  int max_n_file;
  
  int max_reached_storage;
  long int max_reached_n_file;
  int n_replacement;
  int n_victim;
  
  
  long int used_storage;
  int n_saved_file;
  
  int activeReaders;
  int activeWriters;
  pthread_mutex_t mutex;
  pthread_mutex_t ordering;
  pthread_cond_t go;
};

struct thread_args
{
  struct storage* files;
  SharedQueue* work_queue;
  int pipe_fd;
  int* fd_max;
};

struct handler_args
{
  int pipe_fd;
  int* terminate;
};

struct request
{
  char t;
  int fd;
  char* path;
  void* buf;
  int buf_len;
  int flags;
};

struct firstmessage
{
  char op;
  size_t size1;
  size_t size2;
};

#endif
