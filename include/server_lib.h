#if !defined(_SERVER_LIB)
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

#define SEND_FIRST_MESSAGE(message)                                                   \
if(write(*fd, &message, sizeof(struct firstmessage)) !=  sizeof(struct firstmessage)) \
{                                                                                     \
  perror("Writing error");                                                            \
  if(errno == EPIPE)                                                                  \
  {                                                                                   \
    client_disc = 1;                                                                  \
  }                                                                                   \
  else                                                                                \
  {                                                                                   \
    exit(EXIT_FAILURE);                                                               \
  }                                                                                   \
}                                                                                     \
    
#define READ_FIRST_MESSAGE(message)                                                 \
if(read(*fd, &message, sizeof(struct firstmessage)) != sizeof(struct firstmessage)) \
{                                                                                   \
  perror("Reading error");                                                          \
  exit(EXIT_FAILURE);                                                               \
}                                                                                   \

#define READ_PATH(path, size)     \
if(read(*fd, path, size) != size) \
{                                 \
  printf("Invalid message\n");    \
  response.op = 'b';              \
  SEND_FIRST_MESSAGE(response)    \
  free(path);                     \
  continue;                       \
}                                 \
  
struct saved_file
{
  char* name;
  void* buf;
  size_t size;
  int locked;
  int deleting;
  struct timespec last_access;
  fd_set opened;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
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

struct firstmessage
{
  char op;
  size_t size1;
  size_t size2;
};

#endif
