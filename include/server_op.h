#ifndef _SERVER_OP
#define _SERVER_OP

#include <server_lib.h>
#include <lock_lib.h>

int initStorage(struct storage* files, int n_buckets);
int openFile(struct storage* files, int fd, char* path, int flags);
int readFile(struct storage* files, int fd, char* path, void** buf, size_t* size);
int readNFile(struct storage* files, int fd, int N);
int writeFile(struct thread_args* args, int fd, char* path, void* buf, size_t size, char* path_2);
int appendFile(struct thread_args* args, int fd, char* path, void* buf, size_t size, char* path_2);
int lock(struct storage* files, int fd, char* path);
int unlock(struct thread_args* args, int fd, char* path);
int closeFile(struct thread_args* args, int fd, char* path, int fd_max);
int deleteFile(struct storage* files, int fd, char* path, int fd_max);
int replace(struct storage* files, SharedQueue* work_queue);
void scan_hash_r(icl_hash_t* hash, int* bucket, icl_entry_t** curr);
void free_key(void* key);
void free_data(void* data);
int timecmp(struct timespec t1, struct timespec t2);
void closeOpenedFile(struct storage* files, int fd, int fd_max);
int my_icl_hash_dump(FILE* stream, icl_hash_t* ht);

#endif
