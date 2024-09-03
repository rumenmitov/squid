#include "squid.h"

namespace Squid_snapshot {
    using namespace Genode;
    struct Main;
}

void Squid_snapshot::Main::init_squid_file(Path const path, vm_page *me, size_t size) {

    try {
        New_file nf(_root_dir, path);

        if ( nf.append((const char*)payload, size) != New_file::Append_result::OK ) {
            error("couldn't write to squid file: ", path);
        }
 
    } catch (New_file::Create_failed) {
        error("couldn't create squid file: ", path);
    }
}


char* Squid_snapshot::Main::gen_hash() {
    char* timestamp;
    snprintf(timestamp, MD5_HASH_LEN, "%lu", _timer.curr_time());

    char hash[MD5_HASH_LEN];

    /*
     * NOTE:
     * This hashing is only a temporary solution. 
     * In the future replace with either MD5 or sha512.
     */
    strncpy(hash, timestamp, strlen(timestamp));
    for (int i = strlen(timestamp); i < MD5_HASH_LEN - 1; i++) {
        hash[i] = '0';
    }

    hash[MD5_HASH_LEN - 1] = '\0';
    return hash;
}


void Squid_snapshot::Main::read_squid_file(Path const path) {
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
        error("couldn't read squid file: ", path);
    }
}





