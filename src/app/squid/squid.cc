#include "squid.h"

namespace PH_snapshot {
    using namespace Genode;
    struct Main;
}

void PH_snapshot::Main::_new_file(char const *path, void *payload, size_t size) {

    Path p(path);

    try {
        New_file nf(_root_dir, p);

        if ( nf.append((const char*)payload, size) != New_file::Append_result::OK ) {
            error("couldn't write to file!");
        }
 
    } catch (New_file::Create_failed) {
        error("couldn't create file!");
    }
}


void PH_snapshot::Main::_read_file(char const *path) {
    log("reading from file...");
    Path p(path);
    Readonly_file fd(_root_dir, p);
    Readonly_file::At at { 0 };

    char str_buf[1024];
    Byte_range_ptr buffer(str_buf, 1024);

    try {
		for (;;) {

			size_t const read_bytes = fd.read(at, buffer);

            if (strcmp(buffer.start, "hello world!") == 0) {
                log("succ");
            } else {
                log("no succ");
            }


			at.value += read_bytes;

			if (read_bytes < buffer.num_bytes)
				break;
		}
    } catch (...) {
        warning("couldn't read file: ", path);
    }
}



