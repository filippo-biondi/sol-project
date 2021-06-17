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
  long int n = 0;
  size_t nbyte = 0;
  char* endptr;
  char* nstring;
  long int nw;
  void* buf;
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
          if(writeFile(abs_path, NULL) == 0)
          {
            PRINT_OPERATION("File %s written in the server for a total of %ld bytes\n", abs_path, nbyte)
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
      break;
      
    case 'D':
      break;
      
    case 'r':
      dirname = NULL;
      if((ind = find(argc, argv, optind, "-d")) != -1)
      {
        dirname = argv[ind];
        CHECK_PATH(dirname)
        stat(dirname, &st);
        if(!S_ISDIR(st.st_mode))
        {
          errno = ENOTDIR;
          PRINT_OPERATION_ERROR("Bad argument of -d")
          free(abs_dir_path);
          break;
        }
      }
      
      filename = strtok(optarg, ",");
      while(filename != NULL)
      {
        CHECK_PATH(filename)
        if(readFile(filename, &buf, &nbyte) == -1)
        {
          PRINT_OPERATION_ERROR("Error in reading file")
        }
        else
        {
          PRINT_OPERATION("File %s read from the server for a total of %ld bytes\n", filename, nbyte)
          if(dirname != NULL && saveInDir(dirname, filename, buf, nbyte) == 0)
          {
            
            PRINT_OPERATION("File saved in directory %s\n", dirname)
          }
        }
        free(abs_path);
        filename = strtok(NULL, ",");
      }
      if(dirname != NULL)
      {
        free(abs_dir_path);
      }
      break;
    case 'R':
      abs_dir_path = NULL;
      if((ind = find(argc, argv, optind, "-d")) != -1)
      {
        dirname = argv[ind];
        CHECK_PATH(dirname)
        if((abs_dir_path = realpath(dirname, NULL)) == NULL)
        {
          PRINT_OPERATION_ERROR("Error while resolving directory path")
          break;
        }
      }
      
      if(optind != argc && argv[optind][0] != '-')
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
      if((n = readNFiles(n, abs_dir_path)) == -1)
      {
        if(errno == EIO)
        {
          PRINT_OPERATION_ERROR("Error in saving files")
        }
        else
        {
          PRINT_OPERATION_ERROR("Error in reading files")
        }
      }
      else
      {
        PRINT_OPERATION("%ld files read from server\n", n)
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
        if(lockFile(filename) == 0)
        {
          PRINT_OPERATION("Lock acquired on file %s\n", filename)
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
        if(unlockFile(filename) == 0)
        {
          PRINT_OPERATION("Lock released on file %s\n", filename)
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
        if(removeFile(filename) == 0)
        {
          PRINT_OPERATION("File %s removed\n", filename)
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
