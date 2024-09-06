#include "squid.h"
#include <stdio.h>


Squid_snapshot::Error Squid_snapshot::Main::init(char const *path, void *payload, size_t size) {

    try {
        New_file nf(_root_dir, path);

        if ( nf.append((const char*)payload, size) != New_file::Append_result::OK ) {
            return Squid_snapshot::Error::WriteFile;
        }
 
    } catch (New_file::Create_failed) {
            return Squid_snapshot::Error::CreateFile;
    }

    return Squid_snapshot::Error::None;
}


Squid_snapshot::Error Squid_snapshot::Main::get(char const *path, vm_page *payload) {
    if (payload == nullptr) goto NNN;
NNN:

    Readonly_file fd(_root_dir, path);
    Readonly_file::At at { 0 };

    char str_buf[1024];
    Byte_range_ptr buffer(str_buf, 1024);

    try {
		for (;;) {

			size_t const read_bytes = fd.read(at, buffer);

			at.value += read_bytes;

			if (read_bytes < buffer.num_bytes)
				break;
		}

    } catch (...) {
        return Squid_snapshot::Error::ReadFile;
    }

    payload = (vm_page*) ( (void*)buffer.start );
    return Squid_snapshot::Error::None;
}



