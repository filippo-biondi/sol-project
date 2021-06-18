#include <shared_queue.h>
#include <lock_lib.h>

int enqueue(QueueNodePtr* head, QueueNodePtr* tail, void* data)
{
  QueueNodePtr new;
  if((new = malloc(sizeof(QueueNode))) == NULL)
  {
    return -1;
  }
  
  new->data = data;
  new->previous = NULL;
  new->next = *head;
  if(*head != NULL)
  {
    (*head)->previous = new;
  }
  else
  {
    (*tail) = new;
  }
  (*head) = new;
  return 0;
}

int dequeue(QueueNodePtr* head, QueueNodePtr* tail, void** data)
{
  if(*tail != NULL)
  {
    QueueNodePtr last = *tail;
    (*tail) = (*tail)->previous;
    if(*tail != NULL)
    {
      (*tail)->next = NULL;
    }
    else
    {
      *head = NULL;
    }
    *data = last->data;
    free(last);
  
    return 0;
  }
  return -1;
}

int insert_tail(QueueNodePtr* head, QueueNodePtr* tail, void* data)
{
  QueueNodePtr new;
  if((new = malloc(sizeof(QueueNode))) == NULL)
  {
    return -1;
  }
  
  new->data = data;
  new->next = NULL;
  new->previous = *tail;
  if(*tail != NULL)
  {
    (*tail)->next = new;
  }
  else
  {
    *head = new;
  }
  *tail = new;
  return 0;
}

int isempty(QueueNodePtr head)
{
  return head == NULL;
}

void initQueue(SharedQueue* queue)
{
  queue->head = NULL;
  queue->tail = NULL;
  MUTEX_INIT((queue->mutex))
  COND_INIT((queue->empty))
}

int S_enqueue(SharedQueue* queue, void* data)
{
  int ret;
  MUTEX_LOCK(queue->mutex)

  ret = enqueue(&(queue->head), &(queue->tail), data);
  if(ret == 0)
  {
    if(pthread_cond_signal(&(queue->empty)) == -1)
    {
      perror("Error in condition variable");
      return -1;
    }
  }
  MUTEX_UNLOCK(queue->mutex)
  return ret;
}

int S_dequeue(SharedQueue* queue, void** data)
{
  int ret; 
  MUTEX_LOCK(queue->mutex)
  while(isempty(queue->head))
  {
    pthread_cond_wait(&(queue->empty), &(queue->mutex));
  }
  ret = dequeue(&(queue->head), &(queue->tail), data);
  MUTEX_UNLOCK(queue->mutex)
  return ret;
}

int S_insert_tail(SharedQueue* queue, void* data)
{
  int ret;
  MUTEX_LOCK(queue->mutex)

  ret = insert_tail(&(queue->head), &(queue->tail), data);
  if(ret == 0)
  {
    if(pthread_cond_signal(&(queue->empty)) == -1)
    {
      perror("Error in condition variable");
      return -1;
    }
  }
  MUTEX_UNLOCK(queue->mutex)
  return ret;
}
