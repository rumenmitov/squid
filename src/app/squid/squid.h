#pragma once

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

typedef struct vm_page {
    char vpid[32]; // virtual page identification
} vm_page;


struct Squid_snapshot::Main
{
    Env &_env;
    Heap _heap { _env.ram(), _env.rm() };

    Main(Env &env) : _env(env) {}

	Attached_rom_dataspace _config { _env, "config" };

	Vfs::Global_file_system_factory _fs_factory { _heap };

	Vfs::Simple_env _vfs_env { _env, _heap, _config.xml().sub_node("vfs") };

	typedef Directory::Path Path;

	Directory _root_dir { _vfs_env };

    Timer::Connection _timer { _env };

    char* gen_hash();

	void init_squid_file(Path const path, vm_page *me, size_t size);
	void read_squid_file(Path const path, vm_page *me);
	void delete_squid_file(Path const path);
};
