#include <utils.h>
#include <i_conn.h>
#include <conn_supp.h>

extern int print_operation;

void execute_command(int opt, int argc, char* argv[])
{
  char* filename;
  char* dirname;
  char* abs_path = NULL;
  char* abs_dir_path = NULL;
  char* complete_path;
  long int n = 0;
  size_t nbyte = 0;
  char* endptr;
  char* nstring;
  long int nw;
  void* buf;
  int fd;
  int ind;
  struct stat st;
  switch(opt)
  {
    case 'w':
      dirname = strtok(optarg, ",");
      CHECK_PATH(dirname)
      if((abs_dir_path = realpath(dirname, NULL)) == NULL)
      {
        PRINT_OPERATION_ERROR("Error while resolving directory path")
        break;
      }
      if((nstring = strtok(NULL, ",")) != NULL && nstring[0] == 'n' && nstring[1] == '=')
      {
        n = strtol(&(nstring[2]), &endptr, 10);
        if(nstring[2] == '\0' || *endptr != '\0' || n > INT_MAX || n < 0)
        {
          n = 0;
          PRINT_OPERATION("Invalid number as argument of -w, default n=0 value used\n")
        }
      }
      stat(abs_dir_path, &st);
      if(!S_ISDIR(st.st_mode))
      {
        errno = ENOTDIR;
        PRINT_OPERATION_ERROR("Bad argument of -w")
        break;
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
      filename = strtok(optarg, ",");
      ind = find(argc, argv, optind, "-D");
      if(ind != -1)
      {
        dirname = argv[ind];
        stat(dirname, &st);
        if(S_ISDIR(st.st_mode))
        {      
          CHECK_PATH(dirname)
         if((abs_dir_path = realpath(dirname, NULL)) == NULL)
          {
            PRINT_OPERATION_ERROR("Error while resolving directory path")
          }
        }
        else
        {
          errno = ENOTDIR;
          PRINT_OPERATION_ERROR("Bad argument of -D")
        }
      }
      while(filename != NULL)
      {
        CHECK_PATH(filename)
        if((abs_path = realpath(filename, NULL)) == NULL)
        {
          PRINT_OPERATION_ERROR("Error while resolving path")
          free(abs_path);
          abs_dir_path = NULL;
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
        filename = strtok(NULL, ",");
      }
      if(abs_dir_path != NULL)
      {
        free(abs_dir_path);
      }
      break;
      
    case 'D':
      break;
      
    case 'r':
      
      if((ind = find(argc, argv, optind, "-d")) == -1)
      {
        PRINT_OPERATION("No -d option related with -r option\n")
        break;
      }
      dirname = argv[ind];
      CHECK_PATH(dirname)
      if(realpath(dirname, abs_dir_path) == NULL)
      {
        PRINT_OPERATION_ERROR("Error while resolving directory path")
        break;
      }
      stat(dirname, &st);
      if(!S_ISDIR(st.st_mode))
      {
        errno = ENOTDIR;
        PRINT_OPERATION_ERROR("Bad argument of -D")
        free(abs_dir_path);
        break;
      }
      filename = strtok(optarg, ",");
      while(filename != NULL)
      {
        CHECK_PATH(filename)
        if(readFile(abs_path, &buf, &nbyte) == -1)
        {
          PRINT_OPERATION_ERROR("Error in reading file")
        }
        else
        {
          PRINT_OPERATION("File %s read from the server for a total of %ld bytes\n", abs_path, nbyte)
          if((complete_path = get_path(dirname, filename)) == NULL)
          {
            PRINT_OPERATION_ERROR("Error while resolving path")
          }
          CHECK_PATH(complete_path)
          if((fd = open(complete_path, O_WRONLY | O_CREAT | O_TRUNC)) == -1)
          {
            PRINT_OPERATION_ERROR("I/O error in open while saving read file");
          }
          else
          {
            if(write(fd, buf, nbyte) != nbyte)
            {
              PRINT_OPERATION_ERROR("I/O error in write while saving read file"); 
            }
            if(close(fd) == -1)
            {
              PRINT_OPERATION_ERROR("I/O error in close while saving read file"); 
            }
          }
         }
        free(abs_path);
        filename = strtok(NULL, ",");
      }
      free(abs_dir_path);
      break;
    case 'R':
      if((ind = find(argc, argv, optind, "-d")) == -1)
      {
        PRINT_OPERATION("No -d option related with -r option\n")
        break;
      }
      dirname = argv[ind];
      CHECK_PATH(dirname)
      if((abs_dir_path = realpath(dirname, NULL)) == NULL)
      {
        PRINT_OPERATION_ERROR("Error while resolving directory path")
        break;
      }
      if(optind != argc -1 && argv[optind][0] != '-')
      {
        if(argv[optind][0] == 'n' && argv[optind][1] == '=')
        { 
          n = strtol(&(argv[optind][2]), &endptr, 10);
          if(argv[optind][2] == '\0' || *endptr != '\0' || n > INT_MAX || n < 0)
          {
            n = 0;
            PRINT_OPERATION("Invalid number as argument of -R, default n=0 value used\n")
          }
        }
      }
      if((n = readNFiles(n, abs_dir_path)) >= 0)
      {
        PRINT_OPERATION("%ld files read from the server\n", n)
      }
      else
      {
        PRINT_OPERATION_ERROR("Error in reading files") 
      }
      break;
           
    case 'd':
      break;
           
    case 't':
      break;
           
    case 'l':
      filename = strtok(optarg, ",");
      while(filename != NULL)
      {
        CHECK_PATH(filename)
        if(lockFile(abs_path) == 0)
        {
          PRINT_OPERATION("Lock acquired on file %s\n", abs_path)
        }
        else
        {
          PRINT_OPERATION_ERROR("Error while acquiring lock")
        }
        filename = strtok(NULL, ",");
      }
      break;
           
    case 'u':
      filename = strtok(optarg, ",");
      while(filename != NULL)
      {
        CHECK_PATH(filename)
        if(unlockFile(abs_path) == 0)
        {
          PRINT_OPERATION("Lock released on file %s\n", abs_path)
        }
        else
        {
          PRINT_OPERATION_ERROR("Error while releasing lock")
        }
        filename = strtok(NULL, ",");
      }
      break;
           
    case 'c':
      filename = strtok(optarg, ",");
      while(filename != NULL)
      {
        CHECK_PATH(filename)
        if(removeFile(abs_path) == 0)
        {
          PRINT_OPERATION("File %s removed\n", abs_path)
        }
        else
        {
          PRINT_OPERATION_ERROR("Error while removing file")
        }
        filename = strtok(NULL, ",");
      }
      break;
           
    case 'p':
      break;
      
    case 'f':
      break;
    
    case ':':
      PRINT_OPERATION("Missing argument in option -%c\n", optopt)
      break;
           
    case '?':
      PRINT_OPERATION("Unknown option\n")
      break;
  }
}
