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
