#pragma once

#include <base/component.h>
#include <base/sleep.h>
#include <base/attached_rom_dataspace.h>
#include <base/heap.h>
#include <os/vfs.h>
#include <vfs/file_system_factory.h>
#include <base/buffered_output.h>


namespace PH_snapshot {
    using namespace Genode;
    struct Main;
}

typedef struct vm_page {
    int id;
} vm_page;

struct PH_snapshot::Main
{
    Env &_env;
    Heap _heap { _env.ram(), _env.rm() };

    Main(Env &env) : _env(env) {}

	Attached_rom_dataspace _config { _env, "config" };

	Vfs::Global_file_system_factory _fs_factory { _heap };

	Vfs::Simple_env _vfs_env { _env, _heap, _config.xml().sub_node("vfs") };

	typedef Directory::Path Path;

	Directory _root_dir { _vfs_env };

	void _new_file(char const *path, void *payload, size_t size);
	void _read_file(char const *path);
};
