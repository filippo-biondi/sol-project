#include <server_lib.h>
#include <lock_lib.h>

void initStorage(struct storage* files, int n_buckets);
int openFile(struct storage* files, int fd, char* path, int flags);
int readFile(struct storage* files, int fd, char* path, void** buf, size_t* size);
int readNFile(struct storage* files, int fd, int N);
int writeFile(struct storage* files, int fd, char* path, char* path_2);
int appendFile(struct storage* files, int fd, char* path, void* buf, size_t size, char* path_2);
int lock(struct storage* files, int fd, char* path);
int unlock(struct storage* files, int fd, char* path);
int closeFile(struct storage* files, int fd, char* path);
int deleteFile(struct storage* files, int fd, char* path);
int replace(struct storage* files);
void scan_hash_r(icl_hash_t* hash, int* bucket, icl_entry_t** curr);
void free_key(void* key);
void free_data(void* data);
char* get_path(char* dirname, char* filename);
int timecmp(struct timespec t1, struct timespec t2);
