/**
 * @Author Rumen Mitov
 * @Date 2024-09-07

 squid.h provides an API to snapshot data to the disk.
 It is organized as a Radix Tree with an Upper Directory, Middle Directory, and a
 Lower Directory.

 Upper      - determined by the first two chars of the hash.
 Middle     - determined by the next pair of chars of the hash.
 Lower      - determined by the third pair of chars of the hash.
 Squid File - determined by remaining chars in hash.
*/


#ifndef __SQUID_H
#define __SQUID_H

#include <sys/stat.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define HASH_LEN 32

/**
 * @brief Error types associated with Squid Cache.
 */
typedef enum SquidError {
    OpenFile,
    WriteFile,
    ReadFile,
    DeleteFile,
    None
} SquidError_t;

// NOTE: This is only here for testing purposes.
typedef struct vm_page {
    char id[HASH_LEN];
    int testing_secret;
} vm_page;

/**
 * @brief Generates a hash based on timestamp. NEEDS TO BE UPDATED!!!
 * @returns char *
 */
char* squid_generate_hash();

/**
 * @brief Creates squid file.
 * @param const char *
 * @param const void *
 * @param size_t
 * @returns SquidError_t
 */
SquidError_t squid_file_init(char const *path, void *payload, size_t size);

/**
 * @brief Reads from squid file into payload buffer.
 * @param const char *
 * @param void *
 * @returns SquidError_t
 */
SquidError_t squid_file_read(char const *path, void *payload);

/**
 * @brief Deletes squid file from filesystem.
 * @param const char *
 * @returns SquidError_t
 */
SquidError_t squid_file_delete(char const *path);


#endif // __SQUID_H
