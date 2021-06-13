#if !defined(_MY_LIB)
#define _MY_LIB

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

#define MAX_FILE_NO 50 
#define MSEC 10
#define MAX_WAIT_TIME 10
#define MAX_OPEN_DIR 15

#define PRINT_USAGE printf("Usage to be added!!!!!");

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
}
                      
#define MALLOC(ptr, bytes)        \
if((ptr = malloc(bytes)) == NULL) \
{                                 \
  perror("Malloc error");         \
  exit(EXIT_FAILURE);             \
}                                 \

int timecmp(struct timespec t1, struct timespec t2);

#endif
