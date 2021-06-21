#ifndef _UTILS
#define _UTILS

#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 700

#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <limits.h>
#include <time.h>
#include <ftw.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>

#define MAX_FILE_NO 50 
#define MSEC 10
#define MAX_WAIT_TIME 100
#define MAX_OPEN_DIR 15

#define PRINT_USAGE printf("usage: %s -f socketname -w dirname[,n=0] -W file1[,file2] -r file1[,file2] -R [n=0] -d dirname -t time -l file1[,file2] -u file1[,file2] -c file1[,file2] -p\n", argv[0]); \
printf("-f: open connection on socket <socketname>\n-w: write n file from directory <dirname> in the server, if n = 0 write all file from <dirname>\n-W: write in the server a list of file separated by ','\n-r: read from the server a list of file separated by ','\n-R: read n file from the server, if n=0 read all file saved in the server\n-d: directory where file read from the server are stored (must be used after -r or -R)\n-t: time between different request to te server\n-l: acquire lock on a list of file separated by ',' saved in the server\n-u: release lock on a list of file separated by ',' saved in the server\n-c: delete a list of file separated by ',' saved in the server\n-p: enable print of information about operation\n");

#define PRINT_OPERATION(...) \
if(print_operation)          \
{                            \
  printf(__VA_ARGS__);       \
}                            \

#define PRINT_OPERATION_ERROR(str) \
if(print_operation)                \
{                                  \
  perror(str);                     \
}                                  \

#define CHECK_ARGC(n)            \
if(argc < n)                     \
{                                \
  printf("Invalid arguments\n"); \
  return 0;                      \
}                                \
                      
#define MALLOC(ptr, bytes)        \
if((ptr = malloc(bytes)) == NULL) \
{                                 \
  perror("Malloc error");         \
  exit(EXIT_FAILURE);             \
}                                 \

#define CHECK_PATH(pathname)                                        \
if(strnlen(pathname, PATH_MAX) == PATH_MAX)                         \
{                                                                   \
  pathname[PATH_MAX-1] = '\0';                                      \
  PRINT_OPERATION("A pathname was too long and has been truncated") \
}                                                                   \

#endif
