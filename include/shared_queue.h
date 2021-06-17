#if !defined(_SHARED_QUEUE)
#define _SHARED_QUEUE

#include <utils.h>

 struct queue_node
{
  void* data;
  struct queue_node* next;
  struct queue_node* previous;
};
  
typedef struct queue_node QueueNode;
typedef QueueNode* QueueNodePtr;

struct shared_queue
{
  QueueNodePtr head;
  QueueNodePtr tail;
  pthread_mutex_t mutex;
  pthread_cond_t empty;
};

typedef struct shared_queue SharedQueue;

int enqueue(QueueNodePtr* head, QueueNodePtr* tail, void* data);
int dequeue(QueueNodePtr* head, QueueNodePtr* tail, void** data);
int isempty(QueueNodePtr head);

void initQueue(SharedQueue* queue);
int S_enqueue(SharedQueue* queue, void* data);
int S_dequeue(SharedQueue* queue, void** data);


#endif
