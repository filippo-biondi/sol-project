int writeDir(const char* dirname, int n, size_t* nbyte);
int writeFilefn(const char* pathname, const struct stat* info, const int typeflag, struct FTW* pathinfo);
int find(int argc, char* argv[], int optind, char* s);
char* get_path(const char* dirname, char* filename);
