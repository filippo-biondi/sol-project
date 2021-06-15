#include <utils.h>

struct queue_node
  {
    void* data;
    struct queue_node* next;
    struct_queue_node* previous;
  };
  
typedef struct queue_node QueueNode;
typedef QueueNode* QueueNodePtr;

int enqueue(QueueNodePtr* head, void* data);
int dequeue(QueueNodePtr* head, QueueNodePtr* tail, void** data);
int isempty(QueueNodePtr head);

int enqueue(QueueNodePtr* head, void* data)
{
  QueueNodePtr new;
  if((new = malloc(sizeof(QueueNode))) == NULL)
  {
    return -1;
  }
  
  new->data data;
  new->previous = NULL;
  new->next = *head;
  *head->previous = new;
  *head = new;
  return 0;
}

int dequeue(QueueNodePtr* head, QueueNodePtr* tail, void** data)
{
  if(*tail != NULL)
  {
    QuequeNodePtr last = *tail;
    *tail = *tail->previous;
    if(*tail != NULL)
    {
      *tail->next = NULL;
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

int isempty(QueueNodePtr head)
{
  return head == NULL;
}
