#include "include/squidlib.h"
#ifdef __cplusplus

#include <format/snprintf.h>
#include "squid.h"


namespace Squid_snapshot {
    
    SquidFileHash::SquidFileHash(void) 
    {
	l1_dir = 0;
	l2_dir = 0;
	file_id = 0;
    }

    
    SquidFileHash::SquidFileHash(unsigned int availability_matrix[L1_SIZE][L2_SIZE]) 
    {
	for (unsigned int i = 0; i < Squid_snapshot::L1_SIZE; i++) {
	    for (unsigned int j = 0; j < Squid_snapshot::L2_SIZE; j++) {
		if (global_squid->availability_matrix[i][j] < Squid_snapshot::L2_CAP - 1) {
		    this->l1_dir = i;
		    this->l2_dir = j;
		    this->file_id = global_squid->availability_matrix[i][j]++;

		    return;
		}
	    }
	}

	Genode::error("SQUID ERROR: No more hashes available!");
    }


    Genode::Directory::Path SquidFileHash::to_path(void) 
    {
	auto hash_res = Squid_snapshot::global_squid->_heap.try_alloc( sizeof(char) * 20 );
	char *hash = nullptr;
	
	hash_res.with_result(
	    [&](void *addr) { hash = (char*) addr; },
	    [&](Genode::Allocator::Alloc_error err) { Genode::error(err); });

	if (hash == nullptr) Genode::error("memory allocation nullptr");
	
	return Format::snprintf(hash, Squid_snapshot::HASH_LEN, "/squid-cache/%x/%x/%x",
			 l1_dir,
			 l2_dir,
			 file_id);
    }
    


    Main::Main(Env &env) : _env(env) 
    {
	for (unsigned int i = 0; i < Squid_snapshot::L1_SIZE; i++) {
	    char l1_dir[100];
	    Genode::memset(l1_dir, 0, 100);
	    Format::snprintf(l1_dir, 100, "/squid-cache/%x", i);
	    
	    _root_dir.create_sub_directory(l1_dir);
	    if (!_root_dir.directory_exists(l1_dir)) {
		error("ERROR: couldn't create directory");
	    }

	    for (unsigned int j = 0; j < Squid_snapshot::L2_SIZE; j++) {
		char l2_dir[100];
		Genode::memset(l2_dir, 0, 100);
		Format::snprintf(l2_dir, 100, "/squid-cache/%x/%x", i, j);
	    
		_root_dir.create_sub_directory(l2_dir);
		if (!_root_dir.directory_exists(l2_dir)) {
		    error("ERROR: couldn't create directory");
		}

		availability_matrix[i][j] = 0;
	    }
	}
    }


    Error Main::_write(Path const &path, void *payload, size_t size) 
    {
	try {
	    New_file file(_root_dir, path);

	    if ( file.append((const char *) payload, size) != New_file::Append_result::OK )
		return Error::WriteFile;
		  
	} catch (New_file::Create_failed) {
	    return Error::CreateFile;
	}

	return Error::None;
    }

	  
    Error Main::_read(Path const &path, void *payload) 
    {
	Readonly_file file(_root_dir, path);
	Readonly_file::At at { 0 };

	Byte_range_ptr buffer((char*)payload, 1024);

	try {
	    for (;;) {

		size_t const read_bytes = file.read(at, buffer);

		at.value += read_bytes;

		if (read_bytes < buffer.num_bytes)
		    break;
	    }

	} catch (...) {
	    return Error::ReadFile;
	}

	return Error::None;
    }

    Error Main::_delete(Path const &path) 
    {
	
	return Error::None;
    }

    Error Main::_test(void) 
    {
	char message[] = "payload";
	SquidFileHash hash(global_squid->availability_matrix);
	
	switch (Main::_write(hash.to_path(), (void*) message, sizeof(message) / sizeof(char))) {
	case Error::CreateFile:
	    return Error::CreateFile;

	case Error::WriteFile:
	    return Error::WriteFile;

	default:
	    break;
	}
	

	auto echo_res = Squid_snapshot::global_squid->_heap.try_alloc( sizeof(char) * 20 );
	char *echo = nullptr;
	
	echo_res.with_result(
	    [&](void *addr) { echo = (char*) addr; },
	    [&](Genode::Allocator::Alloc_error err) { Genode::error(err); });

	if (echo == nullptr) Genode::error("memory allocation nullptr");
	
	switch (Squid_snapshot::global_squid->_read(hash.to_path(), (void*) echo)) {
	case Error::ReadFile:
	    return Error::ReadFile;

	default:
	    break;
	}

	char *i = echo;
	char *j = message;

	while (*i != 0 && *j != 0) {
	    if (*i != *j) return Error::CorruptedFile;

	    i++;
	    j++;
	}

	if (*i != 0 || *j != 0) return Error::CorruptedFile;

	return Error::None;
    }
    
	  
};



// extern "C" {

//     #include <squidlib.h>
    
//     void squid_hash(char *hash) 
//     {
// 	Squid_snapshot::global_squid->_hash(hash);
//     }


//     enum SquidError squid_write(char const *path, void *payload, unsigned long long size) 
//     {
// 	switch (Squid_snapshot::global_squid->_write(path, payload, size)) {
// 	case Squid_snapshot::Error::CreateFile:
// 	    return SQUID_CREATE;

// 	case Squid_snapshot::Error::WriteFile:
// 	    return SQUID_WRITE;

// 	default:
// 	    return SQUID_NONE;
// 	}
//     }

    
//     enum SquidError squid_read(char const *path, void *payload) 
//     {
// 	switch ( Squid_snapshot::global_squid->_read(path, payload) ) {
// 	case Squid_snapshot::Error::ReadFile:
// 	    return SQUID_READ;

// 	default:
// 	    return SQUID_NONE;
// 	}
//     }
    
//     enum SquidError squid_delete(char const *path) 
//     {
// 	// TODO
// 	(void) path;
// 	return SQUID_NONE;
//     }

//     enum SquidError squid_test(void) 
//     {
// 	switch (Squid_snapshot::global_squid->_test()) {
// 	case Squid_snapshot::Error::CreateFile:
// 	    return SQUID_CREATE;

// 	case Squid_snapshot::Error::WriteFile:
// 	    return SQUID_WRITE;

// 	case Squid_snapshot::Error::ReadFile:
// 	    return SQUID_READ;

// 	case Squid_snapshot::Error::CorruptedFile:
// 	    return SQUID_CORRUPTED;

// 	default:
// 	    return SQUID_NONE;
// 	}
//     }
// }

#endif // __cplusplus
