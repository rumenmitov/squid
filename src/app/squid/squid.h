/**
 * @Author Rumen Mitov
 * @Date 2024-09-07

<<<<<<< HEAD
#include <base/component.h>
#include <base/sleep.h>
#include <base/attached_rom_dataspace.h>
#include <base/heap.h>
#include <os/vfs.h>
#include <vfs/file_system_factory.h>
#include <base/buffered_output.h>
#include <timer_session/connection.h>


/*
 * Squid Structure is as follows:
 * Upper Directory - designated by first two chars of hash
 * Middle Directory - designated by second pair chars of hash
 * Lower Directory - designated by third pair chars of hash
 * Squid files - files that contain virtual page information
 */

#define PAGE_SIZE INT_MAX
#define MD5_HASH_LEN 32

namespace Squid_snapshot {
    using namespace Genode;
    struct Main;
}
=======
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
>>>>>>> unit_tests

#include <sys/stat.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

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
    char vpid[32]; // virtual page identification
} vm_page;

<<<<<<< HEAD

struct Squid_snapshot::Main
{
    Env &_env;
    Heap _heap { _env.ram(), _env.rm() };
=======
/**
 * @brief Creates squid file.
 * @param const char *
 * @param const void *
 * @param size_t
 * @returns SquidError_t
 */
SquidError_t squid_file_init(char const *path, void *payload, size_t size);
>>>>>>> unit_tests

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


<<<<<<< HEAD
	Vfs::Simple_env _vfs_env { _env, _heap, _config.xml().sub_node("vfs") };

	typedef Directory::Path Path;

	Directory _root_dir { _vfs_env };

    Timer::Connection _timer { _env };

    char* gen_hash();

	void init_squid_file(Path const path, vm_page *me, size_t size);
	void read_squid_file(Path const path, vm_page *me);
	void delete_squid_file(Path const path);
};
=======
#endif // __SQUID_H
>>>>>>> unit_tests
