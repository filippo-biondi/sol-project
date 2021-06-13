#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define MAX_FILENAME_LEN 256
#define MAX_DIRNAME_LEN 256
#define MAX_SKTNAME_LEN 256
#define MAX_NUMERIC_STR 13
#define MAX_FILE_NO 50
#define MSEC 10
#define MAX_WAIT_TIME 10
#define PRINT_USAGE printf("?");
#define PRINT_OPERATION(...) if(print_operation){printf(__VA_ARGS__);}

struct message
{
  char[MAX_FILENAME_LEN] file;
  
};

/*struct file_name
{
  char* name;
  char* dirname;
};*/

extern void execute_command(int opt, char* argv[], char* optarg);
int write_dir(dirname, n);
char* find_dir(char* argv[], char* optind);

int print_operation = 0;

int main(int argc, char* argv[])
{
  CHECK_ARGC(?)
  
  int msec = MSEC
  int t = 0;
  char* endptr;
  struct timespec abstime;
  struct timespec sleeptime;
  Filename_nodePtr file_list = NULL;
  char* socket_name;
  for(int i=1; i < argc; i++)
  {
    if(strcmp("-h", argv[i]) == 0)
    {
      PRINT_USAGE
      return 0;
    }
    if(strcmp("-p", argv[i]) == 0)
    {
      print_operation = 1;
    }
    if(strcmp("-t", argv[i]) == 0)
    {
      if(argv[i+1] != NULL && argv[i+1][0] != '-')
      {
        t = strtol(argv[i+1], &endptr, 10);
        if(argv[i+1] == '\0' || **endptr != '\0')
        {
          t = 0;
          PRINT_OPERATION("Invalid argument t, default t=0 setted\n")
        }
      }
      else
      {
        PRINT_OPERATION("Invalid argument t, default t=0 setted\n")
      }
      sleeptime.tv_sec = t / 1000;
      sleeptime.tv_nsec = (t - (sleeptime.tv_sec * 1000)) * 1000;
    }
    if(strcmp("-f", argv[i]) == 0)
    {
      if(socket_name == NULL)
      {
        MALLOC(socket_name, strnlen(argv[i+1], MAX_SKTNAME_LEN) + 1)
        strncpy(socket_name, argv[i+1], MAX_SKTNAME_LEN);
        socket_name[MAX_SKTNAME_LEN] = '\0';
      }
      else
      {
        printf("Too many socket name specified\n");
        PRINT_USAGE
        return 0;
      }
    }
  }
  
  clock_gettime(CLOCK_REALTIME, &abstime);
  abstime.tv_sec += MAX_WAIT_TIME;
  if(openConnection(socket_name, msec, abstime) == -1)
  {
    perror("Connection failed");
    exit(EXIT_FAILURE);
  }
  PRINT_OPERATION("Connection opened on socket %s\n", socket_name)
  
  int opt;
  opt = getopt(argc, argv, "w:W:r:R::d:D:t:l:u:c:");
  while(opt != -1)
  {
    execute_command(opt, argv, optarg);
    nanosleep(sleeptime, NULL);
  }
  if(closeConnection(socket_name) == 0)
  {
    PRINT_OPERATION("Connection successfully closed \n")
  }
  free(soket_name);
  return 0;
}
