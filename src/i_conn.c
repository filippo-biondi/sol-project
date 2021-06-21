#include <utils.h>
#include <i_conn.h>


/* legend for op:
client:                         server:
o: open a file                  y: operation executet correctly
r: read a file                  e: file already exist (only in open)
R: read many file               n: file doesn't exist
w: write a file                 p: permission denied for operation
a: append to a file             t: file too large
l: lock a file                  other: error in communication
u: unlock a file
c: close a file
d: delete a file




*/
int fd_skt;
struct sockaddr_un sa;
int connected = 0;

int openConnection(const char* sockname, int msec, const struct timespec abstime)
{
  struct timespec curr_time;
  struct timespec wait_time;
  
  sa.sun_family = AF_UNIX;
  strncpy(sa.sun_path, sockname, NAME_MAX);
  if((fd_skt = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
  {
    return -1;
  }
   wait_time.tv_sec = msec / 1000;
   wait_time.tv_nsec = (msec % 1000) * 1000000;
  if(clock_gettime(CLOCK_REALTIME, &curr_time) != 0)
  {
    return -1;
  }
  while(timecmp(abstime, curr_time) > 0)
  {
    if(connect(fd_skt, (struct sockaddr*)&sa, sizeof(sa)) == -1)
    {
      nanosleep(&wait_time, NULL);
      if(clock_gettime(CLOCK_REALTIME, &curr_time) != 0)
      {
        return -1;
      }
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
  if(sockname == NULL)
  {
    errno = EINVAL;
    return -1;
  }
  if(sa.sun_path == NULL)
  {
    errno = ENOENT;
    return -1;
  }
  if(connected)
  {
    if(strncmp(sockname, sa.sun_path, NAME_MAX) == 0)
    {
     return close(fd_skt);
    }
    errno = ENOENT;
    return -1;
  }
  else
  {
    errno = ENOTCONN;
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
    message.size1 = (strnlen(pathname, PATH_MAX-1) + 1) * sizeof(char);
    message.size2 = sizeof(int);
    
    SEND_FIRST_MESSAGE(message)
    SEND_PATHNAME(pathname)
    
    if(write(fd_skt, &flags, sizeof(int)) != sizeof(int))
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
      case 'p':
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
int readFile(const char* pathname, void** buf, size_t* size)
{
  long int byte_read = 0;
  struct firstmessage message;
  struct firstmessage response;
  if(connected)
  {
    message.op = 'r';
    message.size1 = (strnlen(pathname, PATH_MAX-1) + 1) * sizeof(char);
    message.size2 = 0;
    SEND_FIRST_MESSAGE(message)
    SEND_PATHNAME(pathname)
    
    READ_RESPONSE(response)
    switch(response.op)
    {
      case 'n':
        errno = ENOENT;
        return -1;
      case 'p':
        errno = EPERM;
        return -1;
      case 'y':
        *size = response.size1;
        if((*buf = malloc(*size)) == NULL)
        {
          return -1;
        }
        while((byte_read += read(fd_skt, *buf + byte_read, *size - byte_read)) != *size) //while loop is necessary for reading large file 
        {
          if(errno != 0)
          {
            errno = ECOMM;
            return -1;
          }
        }
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

int readNFiles(int N, const char* dirname)
{
  size_t len_path;
  size_t len_buf;
  char* path;
  void* buf;
  int files_read = 0;
  int save_err = 0;
  int read_bytes = 0;
  struct firstmessage message;
  struct firstmessage response;
  if(connected)
  {
    message.op = 'R';
    message.size1 = sizeof(int);
    message.size2 = 0;
    SEND_FIRST_MESSAGE(message)
    if(write(fd_skt, &N, sizeof(int)) != sizeof(int)) 
    {                                 
      errno = ECOMM;                  
      return -1;                      
    }
    READ_RESPONSE(response)
    switch(response.op)
    {
      case 'y':
        if(read(fd_skt, &len_path, sizeof(size_t)) != sizeof(size_t) || read(fd_skt, &len_buf, sizeof(size_t)) != sizeof(size_t))
        {
          errno = ECOMM;
          return -1;
        }
        while(len_buf != 0)  //read until server send a firstmessafe with len_buf == 0, this means that sending of file is ended
        {
          if((path = malloc(len_path)) == NULL || (buf = malloc(len_buf)) == NULL)
          {
            return -1;
          }
          errno = 0;
          if(read(fd_skt, path, len_path) != len_path) 
          {
            errno = ECOMM;
            return -1;
          }
          errno = 0;
          while((read_bytes += read(fd_skt, buf + read_bytes, len_buf - read_bytes)) != len_buf)
          {
            if(errno != 0)
            {
              errno = ECOMM;
              return -1;
            }
          }
          read_bytes = 0;
          files_read++;
          if(dirname != NULL && saveInDir(dirname, path, buf, len_buf) == -1)  //try to save file in specified directory
          {
            save_err = 1;
          }
          free(path);
          free(buf);
          
          if(read(fd_skt, &len_path, sizeof(size_t)) != sizeof(size_t) || read(fd_skt, &len_buf, sizeof(size_t)) != sizeof(size_t))
          {
            errno = ECOMM;
            return -1;
          }
        }

        if(save_err == 1)
        {
          return -1;
        }
        return files_read;
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
  long int written_byte = 0;
  FILE* fdW;
  void* buf;
  struct stat st;
  struct firstmessage message;
  struct firstmessage response;
  if(connected)
  {
    message.op = 'w';
    message.size1 = (strnlen(pathname, PATH_MAX-1) + 1) * sizeof(char);
    stat(pathname, &st);
    message.size2 = st.st_size;
    if((buf = malloc(st.st_size)) == NULL)
    {
      return -1;
    }
    if((fdW = fopen(pathname, "rb")) == NULL)  //file is opened and then read from secondary memory
    {
      return -1;
    }
    if(fread(buf, st.st_size, 1, fdW) != 1)
    {
      fclose(fdW);
      return -1;
    }
    fclose(fdW);
    SEND_FIRST_MESSAGE(message)
    SEND_PATHNAME(pathname)
    
    errno = 0;
    while((written_byte += write(fd_skt, buf + written_byte, st.st_size - written_byte)) != st.st_size)  //while loop is necessary for sending large file 
    {
      if(errno != 0)
      {
        break;
      }
    }
    if(errno != 0)
    {
      free(buf);
      errno = ECOMM;
      return -1;
    }
    
    READ_RESPONSE(response)
    switch(response.op)
    {
      case 'y':
        free(buf);
        return 0;
      case 'p':
        free(buf);
        errno = EPERM;
        return -1;
        case 't':
        free(buf);
        errno = EFBIG;
        return -1;
      default:
        free(buf);
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
  long int written_byte = 0;
  if(connected)
  {
    message.op = 'a';
    message.size1 = (strnlen(pathname, PATH_MAX-1) + 1) * sizeof(char);
    message.size2 = size;
    SEND_FIRST_MESSAGE(message)
    SEND_PATHNAME(pathname)
    errno = 0;
    SEND_BUF(buf, size);
    
    READ_RESPONSE(response)
    switch(response.op)
    {
      case 'y':
        return 0;
      case 'p':
        errno = EPERM;
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
    message.size1 = (strnlen(pathname, PATH_MAX-1) + 1) * sizeof(char);
    message.size2 = 0;
    SEND_FIRST_MESSAGE(message)
    SEND_PATHNAME(pathname)
    READ_RESPONSE(response)
    switch(response.op)
    {
      case 'y':
        return 0;
      case 'p':
        errno = EPERM;
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
    message.size1 = (strnlen(pathname, PATH_MAX-1) + 1) * sizeof(char);
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
      case 'p':
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
int closeFile(const char* pathname)
{
  struct firstmessage message;
  struct firstmessage response;
  if(connected)
  {
    message.op = 'c';
    message.size1 = (strnlen(pathname, PATH_MAX-1) + 1) * sizeof(char);
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
      case 'p':
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
int removeFile(const char* pathname)
{
  struct firstmessage message;
  struct firstmessage response;
  if(connected)
  {
    message.op = 'd';
    message.size1 = (strnlen(pathname, PATH_MAX-1) + 1) * sizeof(char);
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
