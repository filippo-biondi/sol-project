#if !defined(_LOCK_LIB)
#define _LOCK_LIB

#include <pthread.h>

#define MUTEX_INIT(mutex)                   \
if(pthread_mutex_init(&(mutex), NULL) != 0) \
{                                           \
  perror("Mutex initalization error");      \
  exit(EXIT_FAILURE);                       \
}                                           \

#define COND_INIT(cond)                             \
if(pthread_cond_init(&(cond), NULL) != 0)           \
{                                                   \
  perror("Condition variable initalization error"); \
  exit(EXIT_FAILURE);                               \
}                                                   \
  
#define MUTEX_LOCK(mutex)             \
if(pthread_mutex_lock(&(mutex)) != 0) \
{                                     \
  perror("Mutex acquire error");      \
  return -1;                          \
}                                     \
  
#define MUTEX_UNLOCK(mutex)             \
if(pthread_mutex_unlock(&(mutex)) != 0) \
{                                       \
  perror("Mutex release error");        \
  exit(EXIT_FAILURE);                   \
}                                       \

#define WRITER_LOCK                                         \
pthread_mutex_lock(&(files->ordering));                     \
pthread_mutex_lock(&(files->mutex));                        \
while(files->activeReaders > 0 || files->activeWriters > 0) \
{                                                           \
  pthread_cond_wait(&(files->go), &(files->mutex));         \
}                                                           \
files->activeWriters++;                                     \
pthread_mutex_unlock(&(files->ordering));                   \
pthread_mutex_unlock(&(files->mutex));                      \


#define WRITER_UNLOCK                  \
pthread_mutex_lock(&(files->mutex));   \
files->activeWriters--;                \
pthread_cond_signal(&(files->go));     \
pthread_mutex_unlock(&(files->mutex)); \
    
#define READER_LOCK                                 \
pthread_mutex_lock(&(files->ordering));             \
pthread_mutex_lock(&(files->mutex));                \
while(files->activeWriters > 0)                     \
{                                                   \
  pthread_cond_wait(&(files->go), &(files->mutex)); \
}                                                   \
files->activeReaders++;                             \
pthread_mutex_unlock(&(files->ordering));           \
pthread_mutex_unlock(&(files->mutex));              \

#define READER_UNLOCK                  \
pthread_mutex_lock(&(files->mutex));   \
files->activeReaders--;                \
if(files->activeReaders == 0)          \
{                                      \
  pthread_cond_signal(&(files->go));   \
}                                      \
pthread_mutex_unlock(&(files->mutex)); \

#endif
