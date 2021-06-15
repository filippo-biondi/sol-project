#include <utils.h>
#include <queue.h>

#define MUTEX_LOCK(mutex)             \
if(pthread_mutex_lock(&(mutex)) != 0) \
  {                                   \
    perror("Mutex acquire error");    \
    return -1;                        \
  }                                   \
  
#define MUTEX_UNLOCK(mutex)             \
if(pthread_mutex_unlock(&(mutex)) != 0) \
    {                                   \
      perror("Mutex release error");    \
      exit(EXIT_FAILURE);               \
    }                                   \
    return -1;                          \
  }                                     \
  
struct shared_queue
{
  QueueNodePtr head;
  QueueNodePtr tail;
  pthread_mutex_lock mutex;
};

typedef shared_queue SharedQueue;

void initQueue(SharedQueue* queue);
int S_enqueue(SharedQueue* queue, void* data);
int S_dequeue(SharedQueue* queue, void** data);

void initQueue(SharedQueue* queue)
{
  queue->head = NULL;
  queue->tail = NULL;
  MUTEX_INIT((queue->mutex))
}

int S_enqueue(SharedQueue* queue, void* data)
{
  int ret; 
  MUTEX_LOCK(queue->mutex)
  ret = enqueue(&(queue->head), data);
  MUTEX_UNLOCK(queue->mutex)
  return ret;
}

int S_dequeue(SharedQueue* queue, void** data)
{
  int ret; 
  MUTEX_LOCK(queue->mutex)
  ret = dequeue(&(queue->head), &(queue->tail), data);
  MUTEX_UNLOCK(queue->mutex)
  return ret;
}

