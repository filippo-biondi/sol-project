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
  int ret = 0;
  
  WRITER_LOCK
  
  if((file = icl_hash_find(files->hashT, path)) == NULL)
  {
    if(flags & O_CREATE)
    {
      if(file != NULL && file->deleting == 1)
      {
        
      }
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
        MUTEX_INIT(file->mutex)
        COND_INIT(file->cond)
        if(icl_hash_insert(files->hashT, path, file) == NULL)
        {
          ret = -1;
        }
      }
    }
    else
    {
      free(path);
      errno = ENOENT;
      ret = -1;
    }
  }
  else
  {
    free(path);
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
        while(file->locked != fd && file->locked != -1)
        {
          pthread_mutex_lock(&(files->mutex)); 
          files->activeWriters--; 
          pthread_cond_signal(&(files->go)); 
          pthread_cond_wait(&(file->cond), &(files->mutex));

          WRITER_LOCK
        }
        if(flags & O_LOCK)
        {
          file->locked = fd;
        }
        FD_SET(fd, &(file->opened));
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
      while(file->locked != fd && file->locked != -1 && files->activeWriters != 0)
      {
        pthread_mutex_lock(&(files->mutex));
        files->activeReaders--; 
        if(files->activeReaders == 0) 
        { 
          pthread_cond_signal(&(files->go)); 
        } 
        pthread_cond_wait(&(file->cond), &(files->mutex));
        
        READER_LOCK
      }
      if((*buf = malloc(file->size)) == NULL)
      {
        ret = -1;
      }
      else
      {
        memcpy(*buf, file->buf, file->size);
        *size = file->size;
        clock_gettime(CLOCK_REALTIME, &(file->last_access));
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
    if(file->locked != fd && file->locked != -1)
    {
      scan_hash_r(files->hashT, &bucket, &curr);
      continue;
    }
    clock_gettime(CLOCK_REALTIME, &(file->last_access));
    name_len = strlen(file->name) + 1;
    buf_size = file->size;
    if(write(fd, &name_len, sizeof(size_t)) != sizeof(size_t) || write(fd, &buf_size, sizeof(size_t)) != sizeof(size_t))
    {
      ret = -1;
      break;
    }
    else
    {
      if(write(fd, file->name, name_len) != name_len || write(fd, file->buf, buf_size) != buf_size)
      {
        ret = -1;
        break;
      }
      ret++;
      scan_hash_r(files->hashT, &bucket, &curr);
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

int writeFile(struct storage* files, int fd, char* path, char* path_2)
{
  int ret = 0;
  int fd_src;
  int temp_vict;
  struct stat st;
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
    else
    {
      if((fd_src = open(path, O_RDONLY)) == -1)
      {
        ret = -1;
      }
      else
      {
        fstat(fd_src, &st);
        if(st.st_size < files->max_storage)
        {
          if((file->buf = malloc(st.st_size)) == NULL ||  read(fd_src, file->buf, st.st_size) != st.st_size)
          {
            ret = -1;
          }
          else
          {
            temp_vict = files->n_victim;
            while(files->used_storage + st.st_size > files->max_storage || files->n_saved_file + 1 > files->max_n_file)
            {
              replace(files);
              files->n_victim++;
            }
            if(temp_vict != files->n_victim)
            {
              files->n_replacement++;
            }
            file->size = st.st_size;
            files->used_storage += st.st_size;
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
            close(fd_src);
          }
        }
        else
        {
          file->deleting = 1;
          errno = EFBIG;
          ret = -1;
        }
      }
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
  
  WRITER_LOCK
  
  if((file = icl_hash_find(files->hashT, path)) == NULL)
  { 
    errno = ENOENT;
    ret = -1;
  }
  else
  { 
    if(size > files->max_storage)
    {
      errno = EFBIG;
      ret = -1;
    }
    else
    {
      if(FD_ISSET(fd, &(file->opened)) == 1)
      {
        while(file->locked != fd || file->locked != -1)
        {
          pthread_mutex_lock(&(files->mutex)); 
          files->activeWriters--; 
          pthread_cond_signal(&(files->go)); 
          pthread_cond_wait(&(file->cond), &(files->mutex));
            
          WRITER_LOCK
        }
        if((file->buf = realloc(file->buf, file->size + size)) == NULL)
        {
          ret = -1;
        }
        else
        {
          endbuf = file->buf + file->size;
          memcpy(endbuf, buf, size);
          if(file-size == 0)
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
      while(file->locked != fd && file->locked != -1)
      {
        pthread_mutex_lock(&(files->mutex)); 
        files->activeWriters--; 
        pthread_cond_signal(&(files->go)); 
        pthread_cond_wait(&(file->cond), &(files->mutex));
          
        WRITER_LOCK
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

int unlock(struct storage* files, int fd, char* path)
{
  int ret = 0;
  struct saved_file* file;
  
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
        pthread_cond_signal(&(file->cond));
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

int closeFile(struct storage* files, int fd, char* path, int fd_max)
{
  int ret = 0;
  int i;
  struct saved_file* file;
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
  
  if((file = icl_hash_find(files->hashT, path)) == NULL)
  { 
    errno = ENOENT;
    ret = -1;
  }
  else
  {
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
    files->n_saved_file --;
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
  files->n_saved_file --;
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
  free(file->name);
  free(file->buf);
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

void closeOpenedFile(struct storage* files, int fd)
{
  int bucket = 0;
  icl_entry_t* curr = NULL;
  struct saved_file* file = NULL;
  
  scan_hash_r(files->hashT, &bucket, &curr);
  
  while(curr != NULL)
  {
    file = (struct saved_file*) curr->data;
    if(FD_ISSET(fd, &(file->opened)) == 1)
    {
      FD_CLR(fd, &(file->opened));
    }
    scan_hash_r(files->hashT, &bucket, &curr);
  }
}
