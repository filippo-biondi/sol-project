#include <utils.h>
#include <i_conn.h>
#include <conn_supp.h>

extern int print_operation;

int execute_command(int opt, int argc, char* argv[])
{
  char* filename;
  char* dirname;
  char* abs_path = NULL;
  long int n = 0;
  size_t nbyte = 0;
  char* endptr;
  char* nstring;
  long int nw;
  void* buf;
  int ind;
  int ret = 0;
  struct stat st;
  switch(opt)
  {
    case 'w':
      dirname = strtok(optarg, ",");
      CHECK_PATH(dirname)
      if((nstring = strtok(NULL, ",")) != NULL && nstring[0] == 'n' && nstring[1] == '=')  //check if number of file is specified and convert it to int
      {
        n = strtol(&(nstring[2]), &endptr, 10);
        if(nstring[2] == '\0' || *endptr != '\0' || n > INT_MAX || n < 0)
        {
          n = 0;
          PRINT_OPERATION("Invalid number as argument of -w, default n=0 value used\n")
        }
      }
      stat(dirname, &st);
      if(!S_ISDIR(st.st_mode))
      {
        errno = ENOTDIR;
        PRINT_OPERATION_ERROR("Bad argument of -w")
        break;
      }
      nw = writeDir(dirname, n, &nbyte);
      if(nw >= 0)
      {
        PRINT_OPERATION("%ld files written in the server from the directory %s for a total of %ld bytes\n", nw, dirname, nbyte)
      }
      else
      {
        PRINT_OPERATION_ERROR("Error while writing directory")
      }
      break;
      
    case 'W':
      filename = strtok(optarg, ",");  //get first file
      
      while(filename != NULL)
      {
        CHECK_PATH(filename)
        if((abs_path = realpath(filename, NULL)) == NULL)
        {
          PRINT_OPERATION_ERROR("Error while resolving path")
          free(abs_path);
          break;
        }
        stat(abs_path, &st);
        nbyte = st.st_size;  //writeFile doesn't return number of byte writte so we get that here
        if(S_ISREG(st.st_mode))  //if is a regular file
        {
          if(openFile(abs_path, O_CREATE | O_LOCK) == 0)
          {
            PRINT_OPERATION("File %s opened in the server\n", abs_path)
            
            if(writeFile(abs_path, NULL) == 0)  //write only if open succeded
            {
              PRINT_OPERATION("File %s written in the server for a total of %ld bytes\n", abs_path, nbyte)
            }
            else
            {
              PRINT_OPERATION_ERROR("Error while writing file")
            }
            if(closeFile(abs_path) == 0) //close even if write failed
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
        }
        else
        {
          PRINT_OPERATION("%s is not a regular file\n", abs_path)
        }
        filename = strtok(NULL, ",");  //get next file
        free(abs_path);
        abs_path = NULL;
      }
      break;
      
    case 'D': //not implemented
      ret = -1;
      break;
      
    case 'r':
      dirname = NULL;
      if((ind = find(argc, argv, optind, 'd')) != -1)  //search for -d after -r
      {
        dirname = argv[ind];
        CHECK_PATH(dirname)
        stat(dirname, &st);
        if(!S_ISDIR(st.st_mode))
        {
          errno = ENOTDIR;
          PRINT_OPERATION_ERROR("Bad argument of -d")
          break;
        }
      }
      
      filename = strtok(optarg, ",");
      while(filename != NULL)
      {
        CHECK_PATH(filename)
        if((abs_path = realpath(filename, NULL)) == NULL)
        {
          PRINT_OPERATION_ERROR("Error while resolving path")
          break;
        }
        if(openFile(abs_path, 0) == 0)
        {
          PRINT_OPERATION("File %s opened in the server\n", abs_path)
          if(readFile(abs_path, &buf, &nbyte) == -1)
          {
            PRINT_OPERATION_ERROR("Error in reading file")
          }
          else
          {
            PRINT_OPERATION("File %s read from the server for a total of %ld bytes\n", abs_path, nbyte)
            if(dirname != NULL && saveInDir(dirname, filename, buf, nbyte) == 0)
            {
              
              PRINT_OPERATION("File saved in directory %s\n", dirname)
            }
            free(buf);
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
        abs_path = NULL;
      }
      break;
    case 'R':
      dirname = NULL;
      if((ind = find(argc, argv, optind, 'd')) != -1)  //search for -d after -R
      {
        dirname = argv[ind];
        CHECK_PATH(dirname)
        stat(dirname, &st);
        if(!S_ISDIR(st.st_mode))
        {
          errno = ENOTDIR;
          PRINT_OPERATION_ERROR("Bad argument of -d")
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
      if((n = readNFiles(n, dirname)) == -1)
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
        if(dirname != NULL)
        {  
          PRINT_OPERATION("File saved in directory %s\n", dirname)
        }
      }
      break;
           
    case 'd':  //check if there is a -r or -R before -d and print error if there isn't
      ret = -1;
      ind = find(argc, argv, 1, 'r');
      if(ind > optind || ind == -1)
      {
        ind = find(argc, argv, 1, 'R');
        if(ind > optind || ind == -1)
        {
          PRINT_OPERATION("No option -r or -R related with option -d\n")
        }
      }
      break;
           
    case 't':  //do nothing, -t already read in main
      ret = -1;
      break;
           
    case 'l':
      filename = strtok(optarg, ",");
      while(filename != NULL)
      {
        CHECK_PATH(filename)
        if((abs_path = realpath(filename, NULL)) == NULL)
        {
          PRINT_OPERATION_ERROR("Error while resolving path")
          free(abs_path);
          break;
        }
        if(openFile(abs_path, 0) == 0)  //open without flags
        {
          PRINT_OPERATION("File %s opened in the server\n", abs_path)
          if(lockFile(abs_path) == 0)
          {
            PRINT_OPERATION("Lock acquired on file %s\n", abs_path)
          }
          else
          {
            PRINT_OPERATION_ERROR("Error while acquiring lock")
          }
        }
        else
        {
          PRINT_OPERATION_ERROR("Error in file opening")
        }
        free(abs_path);
        filename = strtok(NULL, ",");
        abs_path = NULL;
      }
      break;
           
    case 'u':
      filename = strtok(optarg, ",");
      while(filename != NULL)
      {
        CHECK_PATH(filename)
        if((abs_path = realpath(filename, NULL)) == NULL)
        {
          PRINT_OPERATION_ERROR("Error while resolving path")
          free(abs_path);
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
          filename = strtok(NULL, ",");
          abs_path = NULL;
      }
      break;
           
    case 'c':
      filename = strtok(optarg, ",");
      while(filename != NULL)
      {
        CHECK_PATH(filename)
        if((abs_path = realpath(filename, NULL)) == NULL)
        {
          PRINT_OPERATION_ERROR("Error while resolving path")
          free(abs_path);
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
        filename = strtok(NULL, ",");
        abs_path = NULL;
      }
      break;
           
    case 'p':  //do nothing, -p already read in main
      ret = -1;
      break;
      
    case 'f':  //do nothing, -f already read in main
      ret = -1;
      break;
    
    case ':':
      ret = -1;
      PRINT_OPERATION("Missing argument in option -%c\n", optopt)
      break;
           
    case '?':
      ret = -1;
      PRINT_OPERATION("Unknown option\n")
      break;
      
    default:
      ret = -1;
  }
  
  return ret;
}
