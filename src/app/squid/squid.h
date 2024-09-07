#ifndef __SQUID_H
#define __SQUID_H

#include <sys/stat.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

typedef enum SquidError {
    OpenFile,
    WriteFile,
    ReadFile,
    DeleteFile,
    None
} SquidError_t;

typedef struct vm_page {
    int id;
} vm_page;

SquidError_t squid_file_init(char const *path, void *payload, size_t size);

SquidError_t squid_file_read(char const *path, void *payload);

SquidError_t squid_file_delete(char const *path);

#endif // __SQUID_H
