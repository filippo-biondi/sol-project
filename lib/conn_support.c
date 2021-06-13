long int nbyte_read = 0;
long int nwrite = 0;
long int nwrite_tried = 0;
long int nwrite_total;

int writeDir(const char* dirname, int n, long int* nbyte)
{
  nbyte = 0;
  nwrite = 0;
  nwrite_tried = 0;
  nwrite_total = n;
  if(nftw(dirname, writeFilefn, MAX_OPEN_DIR, 0) == -1)
  {
    return -1;
  }
  *nbyte = nbyte_read;
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
        nbyte += info->st_size;
        nwrite_read++;
      }
      closeFile(abs_path)
    }
  }
  if(nwrite_tried == nwrite_total)
  {
    reuturn 1;
  }
}
