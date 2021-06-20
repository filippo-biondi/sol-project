#if !defined(_I_CONN)
#define _I_CONN

#include <utils.h>
#include <conn_supp.h>

#define O_CREATE 1
#define O_LOCK 2

#define SEND_FIRST_MESSAGE(message)                                                      \
if(write(fd_skt, &message, sizeof(struct firstmessage)) !=  sizeof(struct firstmessage)) \
    {                                                                                    \
      errno = ECOMM;                                                                     \
      return -1;                                                                         \
    }                                                                                    \
    
#define SEND_PATHNAME(pathname)                                                                                                    \
if(write(fd_skt, pathname, (strnlen(pathname, PATH_MAX-1) + 1) * sizeof(char)) != (strnlen(pathname, PATH_MAX-1) + 1) * sizeof(char)) \
    {                                                                                                                              \
      errno = ECOMM;                                                                                                               \
      return -1;                                                                                                                   \
    }                                                                                                                              \

#define SEND_DIRNAME(dirname)                                                                                                    \
if(write(fd_skt, dirname, (strnlen(dirname, PATH_MAX-1) + 1) * sizeof(char)) != (strnlen(dirname, PATH_MAX-1) + 1) * sizeof(char)) \
    {                                                                                                                            \
      errno = ECOMM;                                                                                                             \
      return -1;                                                                                                                 \
    }                                                                                                                            \

#define SEND_BUF(buf, size)\
while((written_byte += write(fd_skt, buf + written_byte, size - written_byte)) != size)\
{\
  if(errno != 0)\
  {\
    break;\
  }\
}\
if(errno != 0)\
{\
  errno = ECOMM;\
  return -1;\
}\

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
int timecmp(struct timespec t1, struct timespec t2);
int saveInDir(const char* dirname, char* filename, void* buf, size_t size);

#endif
