struct firstmessage
{
  char op;
  size_t size1;
  size_t size2;
};

int fd_skt;
struct sockaddr_un sa;
int connected = 0;

int openConnection(const char* sockname, int msec, const struct timespec abstime)
{
  struct timespec curr_time;
  struct timespec wait_time;
  
  sa.sun_family = AF_UNIX;
  strncopy(sa.sun_path, sockname, MAX_SKTNAME_LEN);
  if((fd_skt = soket(AF_UNIX, SOCK_STREAM, 0)) == -1)
  {
    return -1;
  }
   wait_time.tv_sec = msec / 1000;
   wait_time.tv_nsec = (msec % 1000) * 1000000;
  if(clock_gettime(CLOCK_REALTIME, &curr_time) != 0)
  {
    return -1;
  }
  while(timecmp(abstime, currr_time) > 0)
  {
    if(connect(fd_skt, (struct sockaddr+)&sa, sizeof(sa)) == -1)
    {
      nanosleep(wait_time, NULL);
    }
    else
    {
      connected = 1;
      return 0;
    }
  }
  errno = ETIMEDOUT;
  return -1;
}

int closeConnection(const char* sockname)
{
  if(sckname == NULL)
  {
    errno = EINVAL;
    return -1;
  }
  if(sa.path == NULL)
  {
    errno = ENOENT;
    return -1;
  }
  if(connected)
  {
    if(strncmp(sockname, sa.path, MAX_SKTNAME_LEN) == 0)
    {
     return close(fd_skt);
    }
    errno = ENOENT;
    return -1;
  }
  else
  {
    errno = ENOTCONN
    return -1;
  }
} 

int openFile(const char* pathname, int flags)
{
  struct firstmessage message;
  struct firstmessage response;
  if(connected)
  {
    message.op = 'o';
    message.size1 = srtnlen(patname, MAX_PATHNAME_LEN) * sizeof(char);
    message.size2 = sizeof(int);
    
    SEND_FIRST_MESSAGE(message);
    SEND_PATHNAME(pathname);
    
    if(write(fd_skt, &flags, sizeof(int))) != sizeof(int))
    {
      errno = ECOMM;
      return -1;
    }
    
    READ_RESPONSE(response)
      
    switch(response.op)
    {
      case 'y':
        return 0;
      case 'e':
        errno = EEXIST;
        return -1;
      case 'n':
        errno = ENOENT;
        return -1;
      default:
        errno = ECOMM;
        return -1;
    }
  }
  else
  {
    errno = ENOTCONN
    return -1;
  }
}
int readFile(const char* pathname, void** buf, size_t* size)
{
  struct firstmessage message;
  struct firstmessage response;
  if(connected)
  {
    message.op = 'r';
    message.size1 = srtnlen(patname, MAX_PATHNAME_LEN) * sizeof(char);
    message.size2 = 0;
    SEND_FIRST_MESSAGE(message)
    SEND_PATHNAME(pathname)
    
    READ_RESPONSE(response)
    switch(response.op)
    {
      case 'n':
        errno = ENOENT;
        return -1;
      case 'y':
        *size = response.size1;
        if((**buff = malloc(*size)) == NULL)
        {
          errno = ENOMEM;
          return -1;
        }
        if(read(fd_skt, *buf, *size) != *size)
        {
          errno = ECOMM;
          return -1;
        }
      default:
        errno = ECOMM;
        return -1;
    }
  }
  else
  {
    errno = ENOTCONN;
    return -1;
  }
}

int readNFiles(int N, const char* dirname)
{
  struct firstmessage message;
  struct firstmessage response;
  if(connected)
  {
    message.op = 'R';
    message.size1 = sizeof(int);
    message.size2 = srtnlen(dirname, MAX_DIRNAME_LEN) * sizeof(char);
    SEND_FIRST_MESSAGE(message)
    SEND_DIRNAME(dirname)
    READ_RESPONSE(response)
    switch(response.op)
    {
      case 'y':
        return 0;
      default:
        errno = ECOMM;
        return -1;
    }
  }
  else
  {
    errno = ENOTCONN;
    return -1;
  }
}

int writeFile(const char* pathname, const char* dirname)
{
  struct firstmessage message;
  struct firstmessage response;
  if(connected)
  {
    message.op = 'w';
    message.size1 = srtnlen(patname, MAX_PATHNAME_LEN) * sizeof(char);
    message.size2 = 0;
    SEND_FIRST_MESSAGE(message)
    SEND_PATHNAME(pathname)
    READ_RESPONSE(response)
    switch(response.op)
    {
      case 'y':
        return 0;
      case 'n':
        errno = EPERM;
        return -1;
      default:
        errno = ECOMM;
        return -1;
    }
  }
  else
  {
    errno = ENOTCONN;
    return -1;
  }
}

int appendToFile(const char* pathname, void* buf, size_t size, const char* dirname)
{
  struct firstmessage message;
  struct firstmessage response;
  if(connected)
  {
    message.op = 'a';
    message.size1 = srtnlen(patname, MAX_PATHNAME_LEN) * sizeof(char);
    message.size2 = size;
    SEND_FIRST_MESSAGE(message)
    SEND_PATHNAME(pathname)
    SEND_BUF(buf, size)
    READ_RESPONSE(response)
    switch(response.op)
    {
      case 'y':
        return 0;
      case 'n':
        errno = ENOENT;
        return -1;
      default:
        errno = ECOMM;
        return -1;
    }
  }
  else
  {
    errno = ENOTCONN;
    return -1;
  }
}
int lockFile(const char* pathname)
{
  struct firstmessage message;
  struct firstmessage response;
  if(connected)
  {
    message.op = 'l';
    message.size1 = srtnlen(patname, MAX_PATHNAME_LEN) * sizeof(char);
    message.size2 = 0;
    SEND_FIRST_MESSAGE(message)
    SEND_PATHNAME(pathname)
    READ_RESPONSE(response)
    switch(response.op)
    {
      case 'y':
        return 0;
      case 'n':
        errno = ENOENT;
        return -1;
      default:
        errno = ECOMM;
        return -1;
    }
  }
  else
  {
    errno = ENOTCONN;
    return -1;
  }
}
int unlockFile(const char* pathname)
{
  struct firstmessage message;
  struct firstmessage response;
  if(connected)
  {
    message.op = 'u';
    message.size1 = srtnlen(patname, MAX_PATHNAME_LEN) * sizeof(char);
    message.size2 = 0;
    SEND_FIRST_MESSAGE(message)
    SEND_PATHNAME(pathname)
    READ_RESPONSE(response)
    switch(response.op)
    {
      case 'y':
        return 0;
      case 'n':
        errno = ENOENT;
        return -1;
      default:
        errno = ECOMM;
        return -1;
    }
  }
  else
  {
    errno = ENOTCONN;
    return -1;
  }
}
int closeFile(const char* pathname)
{
  struct firstmessage message;
  struct firstmessage response;
  if(connected)
  {
    message.op = 'c';
    message.size1 = srtnlen(patname, MAX_PATHNAME_LEN) * sizeof(char);
    message.size2 = 0;
    SEND_FIRST_MESSAGE(message)
    SEND_PATHNAME(pathname)
    READ_RESPONSE(response)
    switch(response.op)
    {
      case 'y':
        return 0;
      case 'n':
        errno = ENOENT;
        return -1;
      default:
        errno = ECOMM;
        return -1;
    }
  }
  else
  {
    errno = ENOTCONN;
    return -1;
  }
}
int removeFile(const char* pathname)
{
  struct firstmessage message;
  struct firstmessage response;
  if(connected)
  {
    message.op = 'd';
    message.size1 = srtnlen(patname, MAX_PATHNAME_LEN) * sizeof(char);
    message.size2 = 0;
    SEND_FIRST_MESSAGE(message)
    SEND_PATHNAME(pathname)
    READ_RESPONSE(response)
    switch(response.op)
    {
      case 'y':
        return 0;
      case 'n':
        errno = ENOENT;
        return -1;
      default:
        errno = ECOMM;
        return -1;
    }
  }
  else
  {
    errno = ENOTCONN;
    return -1;
  }
}
