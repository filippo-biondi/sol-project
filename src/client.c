#include <my_lib.h>
#include <i_conn.h>
extern void execute_command(int opt, int argc, char* argv[]);

int print_operation = 0;

int main(int argc, char* argv[])
{
  CHECK_ARGC(2)
  
  int t = 0;
  char* endptr;
  struct timespec abstime;
  struct timespec sleeptime;
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
        if(argv[i+1][0] == '\0' || *endptr != '\0')
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
        MALLOC(socket_name, strnlen(argv[i+1], NAME_MAX) + 1)
        strncpy(socket_name, argv[i+1], NAME_MAX);
        socket_name[NAME_MAX] = '\0';
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
  if(openConnection(socket_name, MSEC, abstime) == -1)
  {
    perror("Connection failed");
    exit(EXIT_FAILURE);
  }
  PRINT_OPERATION("Connection opened on socket %s\n", socket_name)
  
  int opt;
  opt = getopt(argc, argv, "w:W:r:R::d:D:l:u:c:fhtp");
  while(opt != -1)
  {
    execute_command(opt, argc, argv);
    nanosleep(&sleeptime, NULL);
    opt = getopt(argc, argv, ":w:W:r:R::d:D:l:u:c:fhtp");
  }
  if(closeConnection(socket_name) == 0)
  {
    PRINT_OPERATION("Connection successfully closed \n")
  }
  free(socket_name);
  return 0;
}
