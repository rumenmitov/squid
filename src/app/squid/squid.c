#include "squid.h"

char* squid_generate_hash() 
{
    char *hash = (char*) malloc( sizeof(char) * HASH_LEN );
    if (hash == NULL) return NULL;

    memset(hash, 0, HASH_LEN);
    snprintf(hash, HASH_LEN, "%lu", time(NULL));

    return hash;
}


SquidError_t squid_file_init(char const *path, void *payload, size_t size) 
{
    int fd = open(path, O_CREAT | O_WRONLY);
    if (fd < 0) return OpenFile;

    ssize_t bytes_written = write(fd, payload, size);
    if (bytes_written != size) return WriteFile;

    return None;
}


SquidError_t squid_file_read(char const *path, void *payload) 
{
    int fd = open(path, O_RDONLY);
    if (fd < 0) return OpenFile;

    ssize_t bytes_read = read(fd, payload, 1);
    
    return None;
}


SquidError_t squid_file_delete(char const *path) 
{
    if ( unlink(path) < 0 ) return DeleteFile;

    return None;
}



