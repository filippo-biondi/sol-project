#if !defined(_I_CONN)
#define _I_CONN

#define SEND_FIRST_MESSAGE(message)                                                      \
if(write(fd_skt, &message, sizeof(struct firstmessage)) !=  sizeof(struct firstmessage)) \
    {                                                                                    \
      errno = ECOMM;                                                                     \
      return -1;                                                                         \
    }                                                                                    \
    
#define SEND_PATHNAME(pathname)                                                                                                    \
if(write(fd_skt, patname, srtnlen(patname, MAX_PATHNAME_LEN) * sizeof(char)) != srtnlen(patname, MAX_PATHNAME_LEN) * sizeof(char)) \
    {                                                                                                                              \
      errno = ECOMM;                                                                                                               \
      return -1;                                                                                                                   \
    }                                                                                                                              \

#define SEND_DIRNAME(dirname)                                                                                                    \
if(write(fd_skt, dirname, srtnlen(dirname, MAX_DIRNAME_LEN) * sizeof(char)) != srtnlen(dirname, MAX_DIRNAME_LEN) * sizeof(char)) \
    {                                                                                                                            \
      errno = ECOMM;                                                                                                             \
      return -1;                                                                                                                 \
    }                                                                                                                            \
    
#define SEND_BUF(buf, size)
if(write(fd_skt, buff, size) != size) \
    {                                 \
      errno = ECOMM;                  \
      return -1;                      \
    }                                 \

#define READ_RESPONSE(response)                                                         \
if(read(fd_skt, &response, sizeof(struct firstmessage)) != sizeof(struct firstmessage)) \
    {                                                                                   \ 
      errno = ECOMM;                                                                    \ 
      return -1;                                                                        \ 
    }                                                                                   \

int openConnection(const char* sockname, int msec, const struct timespec abstime);
int closeConnection(const char* sockname);
int openFile(const char* pathname, int flags);
int readFile(const char* pathname, void** buf, size_t* size);
int readNFiles(int N, const char* dirname);
int writeFile(const char* pathname, const char* dirname);
int appendToFile(const char* pathname, void* buf, size_t size, const char* dirname);
int lockFile(const char* pathname);
int unlockFile(const char* pathname);
int closeFile(const char* pathname);
int removeFile(const char* pathname);

#endif
