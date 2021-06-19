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
  if(typeflag == FTW_F)
  {
    nwrite_tried++;
    if(openFile(pathname, O_CREATE | O_LOCK) == 0)
    {
      if(writeFile(pathname, NULL) == 0)
      {
        nbyte_write += info->st_size;
        nwrite++;
      }
      closeFile(pathname);
    }
  }
  if(nwrite_total != 0 && nwrite_tried == nwrite_total)
  {
    return 1;
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

char* get_path(const char* dirname, char* filename)
{
  char* path;
  char* name = basename(filename);
  if((path = malloc((strlen(dirname) + strlen(name) + 2) * sizeof(char))) == NULL)
  {
    errno = ENOMEM;
    return NULL;
  }
  strcpy(path, dirname);
  strcat(path, "/");
  strcat(path, name);
  return path;
}
