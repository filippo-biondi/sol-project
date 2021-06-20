#include <server_op.h>

void initStorage(struct storage* files, int n_buckets)
{
  if((files->hashT = icl_hash_create( n_buckets, hash_pjw, string_compare)) == NULL)
  {
    perror("Error in hash creating");
    exit(EXIT_FAILURE);
  }
  files->max_reached_storage = 0;
  files->max_reached_n_file = 0;
  files->n_replacement = 0;
  files->n_victim = 0;
  files->used_storage = 0;
  files->n_saved_file = 0;
  files->activeWriters = 0;
  files->activeReaders = 0;
  MUTEX_INIT(files->mutex)
  MUTEX_INIT(files->ordering)
  COND_INIT(files->go)
}

int openFile(struct storage* files, int fd, char* path, int flags)
{
  struct saved_file* file;
  struct request* wait_request;
  int ret = 0;
  
  WRITER_LOCK
  
  if((file = icl_hash_find(files->hashT, path)) == NULL)
  {
    if(flags & O_CREATE)
    {
      if((file = malloc(sizeof(struct saved_file))) == NULL || (file->name = malloc(strlen(path) +1 )) == NULL)
      {
        ret = -1;
      }
      else
      {
        strcpy(file->name, path);
        file->buf = NULL;
        file->size = 0;
        file->deleting = 0;
        MALLOC(file->wait_queue, sizeof(SharedQueue));
        initQueue(file->wait_queue);
        MUTEX_INIT(file->mutex)
        clock_gettime(CLOCK_REALTIME, &(file->last_access));
        FD_ZERO(&(file->opened));
        FD_SET(fd, &(file->opened));
        if(flags & O_LOCK)
        {
          file->locked = fd;
        }
        else
        {
          file->locked = -1;
        }
        if(icl_hash_insert(files->hashT, file->name, file) == NULL)
        {
          ret = -1;
        }
      }
    }
    else
    {
      errno = ENOENT;
      ret = -1;
    }
  }
  else
  {
    if(file->deleting == 1)
    {
      errno = EPERM;
      ret = -1;
    }
    else
    {
      if(flags & O_CREATE)
      {
        errno = EEXIST;
        ret = -1;
      }
      else
      {
        if(file->locked != fd && file->locked != -1)
        {
          MALLOC(wait_request, sizeof(struct request))
          wait_request->t = 'o';
          wait_request->fd = fd;
          wait_request->path = path;
          wait_request->buf = NULL;
          wait_request->buf_len = 0;
          wait_request->flags = flags;
          if(S_enqueue(file->wait_queue, (void**) wait_request) == 0)
          {
            errno = EACCES;
          }
          ret = -1;
        }
        else
        {
          if(flags & O_LOCK)
          {
            file->locked = fd;
          }
          FD_SET(fd, &(file->opened));
        }
      }
    }
  }
  
  
  WRITER_UNLOCK
  
  return ret;
}

int readFile(struct storage* files, int fd, char* path, void** buf, size_t* size)
{
  int ret = 0;
  struct saved_file* file;
  struct request* wait_request;
  
  READER_LOCK
  
  if((file = icl_hash_find(files->hashT, path)) == NULL)
  {
    errno = ENOENT;
    ret = -1;
  }
  else
  {
    if(FD_ISSET(fd, &(file->opened)) == 1)
    {
      if(file->locked != fd && file->locked != -1)
      {
        MALLOC(wait_request, sizeof(struct request))
        wait_request->t = 'r';
        wait_request->fd = fd;
        wait_request->path = path;
        wait_request->buf = NULL;
        wait_request->buf_len = 0;
        wait_request->flags = 0;
        if(S_enqueue(file->wait_queue, (void**) wait_request) == 0)
        {
          errno = EACCES;
        }
        ret = -1;
      }
      else
      {
        if((*buf = malloc(file->size)) == NULL)
        {
          ret = -1;
        }
        else
        {
          memcpy(*buf, file->buf, file->size);
          *size = file->size;
          MUTEX_LOCK(file->mutex)
          clock_gettime(CLOCK_REALTIME, &(file->last_access));
          MUTEX_UNLOCK(file->mutex)
        }
      }
    }
    else
    {
      errno = EPERM;
      ret = -1;
    }
  }
  
  READER_UNLOCK
  
  return ret;
}

int readNFile(struct storage* files, int fd, int N)
{
  int ret = 0;
  size_t name_len;
  size_t buf_size;
  int bucket = 0;
  long int written_bytes = 0;
  icl_entry_t* curr = NULL;
  struct saved_file* file;
  struct firstmessage response;
  memset(&response, 0, sizeof(struct firstmessage));
  READER_LOCK
  scan_hash_r(files->hashT, &bucket, &curr);
  response.op = 'y';
  if(write(fd, &response, sizeof(struct firstmessage)) !=  sizeof(struct firstmessage))
  {
    READER_UNLOCK                                                                            
    return -1;                                                            
  } 
  while(curr != NULL && (N == 0 || ret < N))
  {
    file = (struct saved_file*) curr->data;
    if((file->locked != fd && file->locked != -1) || file->deleting == 1 || file->size == 0)
    {
      scan_hash_r(files->hashT, &bucket, &curr);
      continue;
    }
    MUTEX_LOCK(file->mutex)
    clock_gettime(CLOCK_REALTIME, &(file->last_access));
    MUTEX_UNLOCK(file->mutex)
    name_len = strlen(file->name) + 1;
    buf_size = file->size;
    if(write(fd, &name_len, sizeof(size_t)) != sizeof(size_t) || write(fd, &buf_size, sizeof(size_t)) != sizeof(size_t) || write(fd, file->name, name_len) != name_len)
    {
      ret = -1;
    }
    else
    {
      errno = 0;
      while((written_bytes += write(fd, file->buf + written_bytes, buf_size - written_bytes)) != buf_size)
      {
        if(errno != 0)
        {
          ret = -1;
          break;
        }
      }
      if(ret == -1)
      {
        break;
      }
      ret++;
      scan_hash_r(files->hashT, &bucket, &curr);
      written_bytes = 0;
    }
  }
  if(ret != -1)
  {
    name_len = 0;
    buf_size = 0;
    if(write(fd, &name_len, sizeof(size_t)) != sizeof(size_t) || write(fd, &buf_size, sizeof(size_t)) != sizeof(size_t))
    {
      ret = -1;
    }
  }
  
  
  READER_UNLOCK
  
  return ret;
}

int writeFile(struct storage* files, int fd, char* path, void* buf, size_t size, char* path_2)
{
  int ret = 0;
  int temp_vict;
  struct saved_file* file;

  WRITER_LOCK
  
  if((file = icl_hash_find(files->hashT, path)) == NULL)
  {
    errno = ENOENT; 
    ret = -1;
  }
  else
  {
    if(file->size != 0 || file->locked != fd)
    {
      errno = EPERM;
      ret = -1;
    }
        if(size <= files->max_storage)
        {
          if((file->buf = malloc(size)) == NULL)
          {
            ret = -1;
          }
          else
          {
            temp_vict = files->n_victim;
            while(files->used_storage + size > files->max_storage || files->n_saved_file + 1 > files->max_n_file)
            {
              replace(files);
              files->n_victim++;
            }
            if(temp_vict != files->n_victim)
            {
              files->n_replacement++;
            }
            memcpy(file->buf, buf, size);
            file->size = size;
            files->used_storage += size;
            files->n_saved_file++;
            clock_gettime(CLOCK_REALTIME, &(file->last_access));
            
            if(files->used_storage > files->max_reached_storage)
            {
              files->max_reached_storage = files->used_storage;
            }
            if(files->n_saved_file > files->max_reached_n_file)
            {
              files->max_reached_n_file = files->n_saved_file;
            }
          }
        }
        else
        {
          file->deleting = 1;
          errno = EFBIG;
          ret = -1;
        }
  }
  
  WRITER_UNLOCK
  
  return ret;
}

int appendFile(struct storage* files, int fd, char* path, void* buf, size_t size, char* path_2)
{
  int ret = 0;
  void* endbuf;
  int tmplocked;
  int temp_vict;
  struct saved_file* file;
  struct request* wait_request;
  
  WRITER_LOCK
  
  if((file = icl_hash_find(files->hashT, path)) == NULL)
  { 
    errno = ENOENT;
    ret = -1;
  }
  else
  { 
    if(file->size + size > files->max_storage)
    {
      errno = EFBIG;
      ret = -1;
    }
    else
    {
      if(FD_ISSET(fd, &(file->opened)) == 1)
      {
        if(file->locked != fd && file->locked != -1)
        {
          MALLOC(wait_request, sizeof(struct request))
          wait_request->t = 'a';
          wait_request->fd = fd;
          wait_request->path = path;
          wait_request->buf = buf;
          wait_request->buf_len = size;
          wait_request->flags = 0;
          if(S_enqueue(file->wait_queue, (void**) wait_request) == 0)
          {
            errno = EACCES;
          }
          ret = -1;
        }
        else
        {
          if((file->buf = realloc(file->buf, file->size + size)) == NULL)
          {
            ret = -1;
          }
          else
          {
            endbuf = file->buf + file->size;
            memcpy(endbuf, buf, size);
            if(file->size == 0)
            {
              files->n_saved_file++;
            }
            file->size += size;
            files->used_storage += size;
            tmplocked = file->locked;
            file->locked = fd;
            clock_gettime(CLOCK_REALTIME, &(file->last_access));
            temp_vict = files->n_victim;
            while(files->used_storage > files->max_storage || files->n_saved_file > files->max_n_file)
            {
              replace(files);
              files->n_victim++;
            }
            
            if(temp_vict != files->n_victim)
            {
              files->n_replacement++;
            }
            
            file->locked = tmplocked;
            
            if(files->used_storage > files->max_reached_storage)
            {
              files->max_reached_storage = files->used_storage;
            }
            if(files->n_saved_file > files->max_reached_n_file)
            {
              files->max_reached_n_file = files->n_saved_file;
            }
          }
        }
      }
      else
      {
        errno = EPERM;
        ret = -1;
      }
    }
  }
  
  WRITER_UNLOCK
  
  return ret;
}

int lock(struct storage* files, int fd, char* path)
{
  int ret = 0;
  struct saved_file* file;
  struct request* wait_request;
  
  WRITER_LOCK
  
  if((file = icl_hash_find(files->hashT, path)) == NULL)
  { 
    errno = ENOENT;
    ret = -1;
  }
  else
  {
    if(FD_ISSET(fd, &(file->opened)) == 1)
    {
      if(file->locked != fd && file->locked != -1)
      {
        MALLOC(wait_request, sizeof(struct request))
        wait_request->t = 'l';
        wait_request->fd = fd;
        wait_request->path = path;
        wait_request->buf = NULL;
        wait_request->buf_len = 0;
        wait_request->flags = 0;
        if(S_enqueue(file->wait_queue, (void**) wait_request) == 0)
        {
          errno = EACCES;
        }
        ret = -1;
      }
      file->locked = fd;
      clock_gettime(CLOCK_REALTIME, &(file->last_access));
    }
    else
    {
      errno = EPERM;
      ret = -1;
    }
  }
  
  WRITER_UNLOCK
  
  return ret;
}

int unlock(struct thread_args* args, int fd, char* path)
{
  int ret = 0;
  struct storage* files = args->files;
  struct saved_file* file;
  struct request* pending_request;
  
  WRITER_LOCK
  
  if((file = icl_hash_find(files->hashT, path)) == NULL)
  { 
    errno = ENOENT;
    ret = -1;
  }
  else
  {
    if(FD_ISSET(fd, &(file->opened)) == 1)
    {
      if(file->locked != fd && file->locked != -1)
      {
        ret = -1;
        errno = EPERM;
      }
      else
      {
        file->locked = -1;
        clock_gettime(CLOCK_REALTIME, &(file->last_access));
        
        
        while(file->wait_queue->head != NULL)
        {
          if(S_dequeue(file->wait_queue, (void**) &pending_request) == 0)
          {
            S_enqueue(args->work_queue, pending_request);
          }
        }
        
        
      }
    }
    else
    {
      errno = EPERM;
      ret = -1;
    }
  }
  
  WRITER_UNLOCK
  
  return ret;
}

int closeFile(struct thread_args* args, int fd, char* path, int fd_max)
{
  int ret = 0;
  int i;
  struct saved_file* file;
  struct storage* files = args->files;
  struct request* pending_request;
  WRITER_LOCK
  
  if((file = icl_hash_find(files->hashT, path)) == NULL)
  { 
    errno = ENOENT;
    ret = -1;
  }
  else
  {
    FD_CLR(fd, &(file->opened));
    if(file->locked == fd)
    {
      file->locked = -1;
      
      while(file->wait_queue->head != NULL)
      {
        if(S_dequeue(file->wait_queue, (void**) &pending_request) == 0)
        {
          S_enqueue(args->work_queue, pending_request);
        }
      }
    }
    if(file->deleting == 1)
    {
      for(i=0; i <= fd_max; i++)
      {
        if(FD_ISSET(i, &(file->opened)))
        {
          break;
        }
      }
      if(i == fd_max + 1)
      {
        files->used_storage -= file->size;
        if(file->size != 0)
        {
          files->n_saved_file--;
        }
        icl_hash_delete(files->hashT, path, free_key, free_data);
      }
    }
  }
  
  WRITER_UNLOCK
  
  return ret;
}

int deleteFile(struct storage* files, int fd, char* path, int fd_max)
{
  int ret = 0;
  int i;
  struct saved_file* file;
  
  WRITER_LOCK
  
  if((file = icl_hash_find(files->hashT, path)) == NULL || file->deleting == 1)
  { 
    errno = ENOENT;
    ret = -1;
  }
  else
  {
    FD_CLR(fd, &(file->opened));
    if(file->locked == fd)
    {
      file->locked = -1;
    }
    file->deleting = 1;
    for(i=0; i <= fd_max; i++)
    {
      if(FD_ISSET(i, &(file->opened)))
      {
        break;
      }
    }
    if(i == fd_max + 1)
    {
      files->used_storage -= file->size;
      files->n_saved_file--;
      icl_hash_delete(files->hashT, path, free_key, free_data);
    }
  }
  
  WRITER_UNLOCK
  
  return ret;
}

int replace(struct storage* files)
{
  int bucket = 0;
  icl_entry_t* curr = NULL;
  icl_entry_t* victim;
  struct timespec victim_time;
  victim_time.tv_sec = 0;
  struct saved_file* file = NULL;
  
  scan_hash_r(files->hashT, &bucket, &curr);
  
  while(curr != NULL)
  {
    file = (struct saved_file*) curr->data;
    if(file->locked == -1 && (timecmp(file->last_access, victim_time) < 0 || victim_time.tv_sec == 0) && file->size != 0)
    {
      victim = curr;
      victim_time = file->last_access;
    }
    scan_hash_r(files->hashT, &bucket, &curr);
  }
  if(victim != NULL)
  {
    file = (struct saved_file*) victim->data;
    files->used_storage -= file->size;
    files->n_saved_file--;
    icl_hash_delete(files->hashT, file->name, free_key, free_data);
    return 0;
  }
  
  curr = NULL;
  scan_hash_r(files->hashT, &bucket, &curr);
  
  while(curr != NULL)
  {
    file = (struct saved_file*) curr->data;
    if(timecmp(file->last_access, victim_time) < 0 || victim_time.tv_sec == 0)
    {
      victim = curr;
      victim_time = file->last_access;
    }
    scan_hash_r(files->hashT, &bucket, &curr);
  }
  
  if(victim == NULL)
  {
    return -1;
  }
  
  file = (struct saved_file*) victim->data;
  files->used_storage -= file->size;
  files->n_saved_file--;
  icl_hash_delete(files->hashT, file->name, free_key, free_data);
  return 0;
}

void scan_hash_r(icl_hash_t* hash, int* bucket, icl_entry_t** curr)
{
  if(*curr == NULL)
  {
    *bucket = 0;
    for(int i=0; i < hash->nbuckets; i++)
    {
      if(hash->buckets[i] != NULL)
      {
        *curr = hash->buckets[i];
        *bucket = i;
        return;
      }
    }
    return;
  }
  
  for(int i = *bucket; i < hash->nbuckets; i++)
  {
    while((*curr) != NULL && (*curr)->next != NULL)
    {
      *curr = (*curr)->next;
      return;
    }
    if(i != hash->nbuckets-1)
    {
      *curr = hash->buckets[i+1];
      if(*curr != NULL)
      {
        *bucket = i+1;
        return;
      }
    }
  }
  *curr = NULL;
}

void free_key(void* key)
{
  free(key);
}
void free_data(void* data)
{
  struct saved_file* file = (struct saved_file*) data;
  struct request* tmp_req;
  free(file->buf);
  while(file->wait_queue->head != NULL)
  {
    S_dequeue(file->wait_queue, (void**) &tmp_req);
    free(tmp_req);
  }
  free(file->wait_queue);
  free(file);
}

char* get_path(char* dirname, char* filename)
{
  char* path;
  if((path = malloc((strlen(dirname) + strlen(filename) + 2) * sizeof(char))) == NULL)
  {
    errno = ENOMEM;
    return NULL;
  }
  strcpy(path, dirname);
  path[strlen(dirname)] = '/';
  path[strlen(dirname) + 1] = '\0';
  strcat(path, filename);
  return path;
}

int timecmp(struct timespec t1, struct timespec t2)
{
    if (t1.tv_sec == t2.tv_sec)
        return t1.tv_nsec - t2.tv_nsec;
    else
        return t1.tv_sec - t2.tv_sec;
}

void closeOpenedFile(struct storage* files, int fd, int fd_max)
{
  int bucket = 0;
  int i;
  icl_entry_t* curr = NULL;
  struct saved_file* file = NULL;
  
  scan_hash_r(files->hashT, &bucket, &curr);
  
  while(curr != NULL)
  {
    file = (struct saved_file*) curr->data;
    if(FD_ISSET(fd, &(file->opened)) == 1)
    {
      FD_CLR(fd, &(file->opened));
      if(file->locked == fd)
      {
        file->locked = -1;
      }
      if(file->deleting == 1)
      {
        for(i=0; i <= fd_max; i++)
        {
          if(FD_ISSET(i, &(file->opened)))
          {
            break;
          }
        }
        if(i == fd_max + 1)
        {
          files->used_storage -= file->size;
          files->n_saved_file--;
          icl_hash_delete(files->hashT, file->name, free_key, free_data);
        }
      }
    }
    scan_hash_r(files->hashT, &bucket, &curr);
  }
}

int my_icl_hash_dump(FILE* stream, icl_hash_t* ht)
{
    icl_entry_t *bucket, *curr;
    int i;
    fprintf(stream, "Files in storage:\n");
    if(!ht) return -1;

    for(i=0; i<ht->nbuckets; i++) {
        bucket = ht->buckets[i];
        for(curr=bucket; curr!=NULL; ) {
            if(curr->key)
                fprintf(stream, "%s: %ldB\n", (char *)curr->key, ((struct saved_file*) curr->data)->size);
            curr=curr->next;
        }
    }

    return 0;
}
