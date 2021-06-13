#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <limits.h>

#define PRINT_USAGE printf("Usage to be added!!!!!");

#define PRINT_OPERATION(...) \
if(print_operation)          \
{                            \
  printf(__VA_ARGS__);       \
}                            \

PRINT_OPERATION_ERROR(str) \
if(print_operation)        \
{                          \
  perror(str);             \
}                          \

#define CHECK_ARGC(n)            \
if(argc < n)                     \
{                                \
  printf("Invalid arguments\n"); \
  return 0;                      \
}
                      
#define MALLOC(ptr, bytes)\
if((ptr = malloc(bytes)) == NULL) \
{                                 \
  perror("Malloc error");         \
  exit(EXIT_FAILURE);             \
}                                 \

int timecmp(timespec t1, timespec t2);
