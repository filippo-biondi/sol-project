#include <utils.h>
#include <i_conn.h>
#include <conn_supp.h>

long int nbyte_write = 0;
long int nwrite = 0;
long int nwrite_tried = 0;
long int nwrite_total;

int writeDir(const char* dirname, int n, size_t* nbyte)
{
  nbyte_write = 0;
  nwrite = 0;
  nwrite_tried = 0;
  nwrite_total = n;
  if(nftw(dirname, writeFilefn, MAX_OPEN_DIR, 0) == -1)
  {
    return -1;
  }
  *nbyte = nbyte_write;
  return nwrite;
}

int writeFilefn(const char* pathname, const struct stat* info, const int typeflag, struct FTW* pathinfo)
{
  char* abs_path;
  if(typeflag == FTW_F)  //check if is a regular file
  {
    if((abs_path = realpath(pathname, NULL)) == NULL)
    {
      return 0;
    }
    nwrite_tried++;
    if(openFile(abs_path, O_CREATE | O_LOCK) == 0)
    {
      errno = 0;
      if(writeFile(abs_path, NULL) == 0)
      {
        nbyte_write += info->st_size;
        nwrite++;
        closeFile(abs_path);
      }
      if(errno == EFBIG)
      {
        closeFile(abs_path);
      }
    }
  }
  if(nwrite_total != 0 && nwrite_tried == nwrite_total)
  {
    return 1;  //this cause nftw to stop
  }
  return 0;
}

int find(int argc, char* argv[], int optind, char s)
{
  for(int i=optind; i < argc -1; i++)
  {
    if(argv[i][0] == '-' && argv[i][1] == s)
    {
      return i+1;
    }
  }
  return -1;
}

char* get_path(const char* dirname, char* filename)  //merge together a directory path and a file path
{
  char* path;
  char* name = basename(filename);
  if((path = malloc((strlen(dirname) + strlen(name) + 2) * sizeof(char))) == NULL)
  {
    return NULL;
  }
  strcpy(path, dirname);
  if(path[strlen(dirname)] == '/')  //avoid writing another slash if is already present (would work either)
  {
    path[strlen(dirname)] = '\0';
  }
  strcat(path, "/");
  strcat(path, name);
  return path;
}

int saveInDir(const char* dirname, char* filename, void* buf, size_t size)
{
  char* complete_path;
  int fd;
  if((complete_path = get_path(dirname, filename)) == NULL)
  {
    return -1;
  }
  if((fd = open(complete_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXO)) == -1)
  {
    free(complete_path);
    return -1;
  }
  if(write(fd, buf, size) != size)
  {
    free(complete_path);
    return -1;
  }
  if(close(fd) == -1)
  {
    free(complete_path);
    return -1;
  }
  free(complete_path);
  return 0;
}

int timecmp(struct timespec t1, struct timespec t2)
{
    if (t1.tv_sec == t2.tv_sec)
        return t1.tv_nsec - t2.tv_nsec;
    else
        return t1.tv_sec - t2.tv_sec;
}
