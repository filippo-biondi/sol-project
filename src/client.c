#include <utils.h>
#include <i_conn.h>
extern int execute_command(int opt, int argc, char* argv[]);

int print_operation = 0;

int main(int argc, char* argv[])
{
  CHECK_ARGC(2)
  
  int t = 0;
  char* endptr;
  struct timespec abstime;
  struct timespec sleeptime;
  char* socket_name = NULL;
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
        if(i != argc-1)
        {
          size_t socket_len = strnlen(argv[i+1], NAME_MAX) + 1;
          MALLOC(socket_name, socket_len)
          strncpy(socket_name, argv[i+1], socket_len);
          socket_name[socket_len - 1] = '\0';
        }
        else
        {
          printf("No socket name specified\n");
          PRINT_USAGE
          return 0;
        }
      }
      else
      {
        printf("Too many socket name specified\n");
        PRINT_USAGE
        return 0;
      }
    }
  }
  if(socket_name == NULL)
  {
    printf("Option -f not specified, -h for usage\n"); 
  }
  PRINT_OPERATION("Trying connect on %s socket\n", socket_name)
  clock_gettime(CLOCK_REALTIME, &abstime);
  abstime.tv_sec += MAX_WAIT_TIME;
  if(openConnection(socket_name, MSEC, abstime) == -1)
  {
    perror("Connection failed");
    exit(EXIT_FAILURE);
  }
  PRINT_OPERATION("Connection opened on socket %s\n", socket_name)
  
  int opt;
  opt = getopt(argc, argv, "-:w:W:r:Rd:D:l:u:c:fhtp");
  while(opt != -1)
  {
    if(execute_command(opt, argc, argv) == 0 && optind < argc - 1)
    {
      nanosleep(&sleeptime, NULL);
    }
    opt = getopt(argc, argv, ":w:W:r:R::d:D:l:u:c:fhtp");
  }
  if(closeConnection(socket_name) == 0)
  {
    PRINT_OPERATION("Connection successfully closed \n")
  }
  free(socket_name);
  return 0;
}
