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

void execute_command(int opt, char* argv[], char* optarg);
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
  free(soket_name);
  return 0;
}

void execute_command(int opt, char* argv[], char* optarg, char* optind)
{
  char* filename;
  char* dirname;
  long int n = 0;
  long int nbyte = 0;
  char* endptr;
  struct stat st;
  opterr = 0;
  switch(opt)
  {
    case 'w':  
      char nstring;
      long int nw;
      dirname = strtok(dirname, ',');
      if((nstring = strtok(NULL, ',')= != NULL && nstring[0] == 'n' && nstring[1] == '=')
      {
        n = strtol(&(nstring[2]), &endptr, 10);
        if(nstring[2] == '\0' || **endptr != '\0')
        {
          n = 0;
          PRINT_OPERATION("Invalid number as argument of -w, default n=0 value used\n")
        }
      }
            
      nw = write_dir(dirname, n, &nbyte);
      PRINT_OPERATION("%ld files written in the server from the directory %s for a total of %ld bytes\n", nw, dirname, nbyte)
      break;
      
    case 'W':
      filename = strtok(optarg, ',');
      while((filename = strtok(NULL, ',')) != NULL)
      {
        stat(filename, &st);
        nbyte = st.st_size;
        if(openFile(filename, O_CREATE | O_LOCK) == 0)
        {
          PRINT_OPERATION("File %s opened in the server\n", filename)
          if(writeFile(filename, NULL) == 0)
          {
            PRINT_OPERATION("File %s written in the server for a total of %ld bytes\n", filename, nbyte)
          }
          if(closeFile(filename) == 0)
          {
            PRINT_OPERATION("File %s closed\n", filename)
          }
        }
      }
      break;
    case 'D':
      break;
    case 'r':
      char* buf;
      int fd;
      if((dirname = find_dir(argv, optind)) == NULL)
      {
        PRINT_OPERATION("No -d option related with -r option")
        break;
      } 
      filename = strtok(optarg, ',');
      while((filename = strtok(NULL, ',')) != NULL)
      {
         
        if(readFile(filename, &buf, &nbyte) == 0)
        {
          PRINT_OPERATION("File %s read from the server for a total of %ld bytes\n", filename, nbyte)
        }
        if((fd = open(get_path(dirname, filename), O_WRONLY | O_CREAT | O_TRUNC)) == -1)
        {
          perror("I/O error in open while saving read file");
        }
        else
        {
          if(write(fd, buf, size) != size)
          {
            perror("I/O error in write while saving read file"); 
          }
          if(close(fd) == -1)
          {
            perror("I/O error in close while saving read file"); 
          }
        }
      }
      break;
    case 'R':
      if(optarg != 0)
      {
        if((optarg[0] == 'n' && optarg[1] == '=')
        { 
          n = strtol(&(optarg[2]), &endptr, 10);
          if(nstring[2] == '\0' || **endptr != '\0')
          {
            n = 0;
            PRINT_OPERATION("Invalid number as argument of -w, default n=0 value used\n")
          }
        }
      }
      if((n = readNFiles(n, dirname)) >= 0)
      {
        PRINT_OPERATION("%ld files read from the server\n", n)
      }
      break;
    case 'd':
      break;
    case 't':
      break;
    case 'l':
      break;
    case 'u':
      break;
      case 'c':
      break;
    case 'p':
      break;
  }
  opterr = 1
}
