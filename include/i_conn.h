#if !defined(_I_CONN)
#define _I_CONN

#include <utils.h>

#define O_CREATE 10
#define O_LOCK 11

#define SEND_FIRST_MESSAGE(message)                                                      \
if(write(fd_skt, &message, sizeof(struct firstmessage)) !=  sizeof(struct firstmessage)) \
    {                                                                                    \
      errno = ECOMM;                                                                     \
      return -1;                                                                         \
    }                                                                                    \
    
#define SEND_PATHNAME(pathname)                                                                                                    \
if(write(fd_skt, pathname, strnlen(pathname, PATH_MAX) * sizeof(char)) != strnlen(pathname, PATH_MAX) * sizeof(char)) \
    {                                                                                                                              \
      errno = ECOMM;                                                                                                               \
      return -1;                                                                                                                   \
    }                                                                                                                              \

#define SEND_DIRNAME(dirname)                                                                                                    \
if(write(fd_skt, dirname, strnlen(dirname, PATH_MAX) * sizeof(char)) != strnlen(dirname, PATH_MAX) * sizeof(char)) \
    {                                                                                                                            \
      errno = ECOMM;                                                                                                             \
      return -1;                                                                                                                 \
    }                                                                                                                            \
    
#define SEND_BUF(buf, size)           \
if(write(fd_skt, buf, size) != size) \
    {                                 \
      errno = ECOMM;                  \
      return -1;                      \
    }                                 \

#define READ_RESPONSE(response)                                                         \
if(read(fd_skt, &response, sizeof(struct firstmessage)) != sizeof(struct firstmessage)) \
    {                                                                                   \
      errno = ECOMM;                                                                    \
      return -1;                                                                        \
    }                                                                                    \

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
int timecmp(struct timespec t1, struct timespec t2);

#endif
