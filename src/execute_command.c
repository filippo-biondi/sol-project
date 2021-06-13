void execute_command(int opt, int argc, char* argv[])
{
  char* filename;
  char* dirname;
  char* abs_path = NULL;
  char* abs_dir_path = NULL;
  long int n = 0;
  long int nbyte = 0;
  char* endptr;
  struct stat st;
  switch(opt)
  {
    case 'w':  
      char nstring;
      long int nw;
      dirname = strtok(dirname, ',');
      CHECK_PATH(dirname)
      if(realpath(dirname, abs_dir_path) == NULL)
      {
        PRINT_OPERATION_ERROR("Error while resolving directory path")
        break;
      }
      if((nstring = strtok(NULL, ',')) != NULL && nstring[0] == 'n' && nstring[1] == '=')
      {
        n = strtol(&(nstring[2]), &endptr, 10);
        if(nstring[2] == '\0' || **endptr != '\0' || n > INT_MAX || n < 0)
        {
          n = 0;
          PRINT_OPERATION("Invalid number as argument of -w, default n=0 value used\n")
        }
      }
      
      nw = writeDir(abs_dir_path, n, &nbyte);
      if(nw >= 0)
      {
        PRINT_OPERATION("%ld files written in the server from the directory %s for a total of %ld bytes\n", nw, dirname, nbyte)
      }
      else
      {
        PRINT_OPERATION_ERROR("Error while writing directory")
      }
      free(abs_dir_path);
      break;
      
    case 'W':
      filename = strtok(optarg, ',');
      dirname = find-D(argc, argv);
      CHECK_PATH(dirname)
      if(realpath(dirname, abs_dir_path) == NULL)
      {
        PRINT_OPERATION_ERROR("Error while resolving directory path")
      }
      while(filename != NULL)
      {
        CHECK_PATH(filename)
        if(realpath(filename, abs_path) == NULL)
        {
          PRINT_OPERATION_ERROR("Error while resolving path")
          break;
        }
        stat(abs_path, &st);
        nbyte = st.st_size;
        if(openFile(abs_path, O_CREATE | O_LOCK) == 0)
        {
          PRINT_OPERATION("File %s opened in the server\n", abs_path)
          if(writeFile(abs_path, abs_dir_path) == 0)
          {
            PRINT_OPERATION("File %s written in the server for a total of %ld bytes\n", abs_path, nbyte)
            if(abs_dir_path != NULL)
            {
              PRINT_OPERATION("Eventualy discarded files saved in %s\n", abs_dir_path)
            }
          }
          else
          {
            PRINT_OPERATION_ERROR("Error while writing file")
          }
          if(closeFile(abs_path) == 0)
          {
            PRINT_OPERATION("File %s closed\n", abs_path)
          }
          else
          {
            PRINT_OPERATION_ERROR("Error while closing file")
          }
        }
        else
        {
          PRINT_OPERATION_ERROR("Error in file opening")
        }
        free(abs_path);
        filename = strtok(NULL, ',');
      }
      free(abs_dir_path);
      break;
      
    case 'D':
      break;
      
    case 'r':
      char* buf;
      int fd;
      if((dirname = find-d(argc, argv)) == NULL)
      {
        PRINT_OPERATION("No -d option related with -r option\n")
        break;
      }
      CHECK_PATH(dirname)
      if(realpath(dirname, abs_dir_path) == NULL)
      {
        PRINT_OPERATION_ERROR("Error while resolving directory path")
        break;
      }
      filename = strtok(optarg, ',');
      while(filename != NULL)
      {
        CHECK_PATH(filename)
        if(realpath(filename, abs_path) == NULL)
        {
          PRINT_OPERATION_ERROR("Error while resolving path")
          break;
        }
        if(readFile(abs_path, &buf, &nbyte) == 0)
        {
          PRINT_OPERATION("File %s read from the server for a total of %ld bytes\n", abs_path, nbyte)
        }
        char* complete_path = get_path(dirname, filename);
        CHECK_PATH(compete_path)
        if((fd = open(complete_path, O_WRONLY | O_CREAT | O_TRUNC)) == -1)
        {
          PRINT_OPERATION_ERROR("I/O error in open while saving read file");
        }
        else
        {
          if(write(fd, buf, size) != size)
          {
            PRINT_OPERATION_ERROR("I/O error in write while saving read file"); 
          }
          if(close(fd) == -1)
          {
            PRINT_OPERATION_ERROR("I/O error in close while saving read file"); 
          }
        }
        free(abs_path);
        filename = strtok(NULL, ',');
      }
      free(abs_dir_path);
      break;
    case 'R':
      if(optarg != 0)
      {
        if((optarg[0] == 'n' && optarg[1] == '=')
        { 
          n = strtol(&(optarg[2]), &endptr, 10);
          if(nstring[2] == '\0' || **endptr != '\0' || n > MAX_INT || n < 0)
          {
            n = 0;
            PRINT_OPERATION("Invalid number as argument of -R, default n=0 value used\n")
          }
        }
      }
      if((n = readNFiles(n, dirname)) >= 0)
      {
        PRINT_OPERATION("%ld files read from the server\n", n)
      }
      else
      {
        PRINT_OPERATION_ERROR("Error in reading file") 
      }
      break;
           
    case 'd':
      break;
           
    case 't':
      break;
           
    case 'l':
      filename = strtok(optarg, ',');
      while(filename != NULL)
      {
        CHECK_PATH(filename)
        if(realpath(filename, abs_path) == NULL)
        {
          PRINT_OPERATION_ERROR("Error while resolving path")
          break;
        }
        if(lockFile(abs_path) == 0)
        {
          PRINT_OPERATION("Lock acquired on file %s\n", abs_path)
        }
        else
        {
          PRINT_OPERATION_ERROR("Error while acquiring lock")
        }
        free(abs_path);
        filename = strtok(NULL, ',');
      }
      break;
           
    case 'u':
      filename = strtok(optarg, ',');
      while(filename != NULL)
      {
        CHECK_PATH(filename)
        if(realpath(filename, abs_path) == NULL)
        {
          PRINT_OPERATION_ERROR("Error while resolving path")
          break;
        }
        if(unlockFile(abs_path) == 0)
        {
          PRINT_OPERATION("Lock released on file %s\n", abs_path)
        }
        else
        {
          PRINT_OPERATION_ERROR("Error while releasing lock")
        }
        free(abs_path);
        filename = strtok(NULL, ',');
      }
      break;
           
    case 'c':
      filename = strtok(optarg, ',');
      while(filename != NULL)
      {
        CHECK_PATH(filename)
        if(realpath(filename, abs_path) == NULL)
        {
          PRINT_OPERATION_ERROR("Error while resolving path")
          break;
        }
        if(removeFile(abs_path) == 0)
        {
          PRINT_OPERATION("File %s removed\n", abs_path)
        }
        else
        {
          PRINT_OPERATION_ERROR("Error while removing file")
        }
        free(abs_path);
        filename = strtok(NULL, ',');
      }
      break;
           
    case 'p':
      break;
    
    case ':':
      PRINT_OPERATION("Missing argument\n", abs_path)
      break;
           
    case '?':
      PRINT_OPERATION("Unknown option\n", abs_path)
      break;
  }
}
