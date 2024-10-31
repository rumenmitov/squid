#include "include/squidlib.h"
#ifdef __cplusplus

#include "squid.h"


namespace Squid_snapshot {

    void Main::_hash(char *me) 
    {
	me = nullptr;
	
	auto hash_res = this->_heap.try_alloc( sizeof(char) * HASH_LEN );
	
	hash_res.with_result(
	    [&](void *addr) { me = (char*) addr; },
	    [&](Allocator::Alloc_error err) { error(err); });

	if (me == nullptr) error("hash allocation == nullptr");

	for (int i = 0; i < 32; i++) {
	    me[i] = 0;
	}
	
	copy_cstring(me, "thisisasamplehash", 32);
    }

    Error Main::_write(char const *path, void *payload, size_t size) 
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

	  
    Error Main::_read(char const *path, void *payload) 
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

    Error Main::_delete(char const *path) 
    {
	// TODO
	(void) path;
	return Error::None;
    }

    Error Main::_test(void) 
    {
	char message[20] = "payload";
	switch (Main::_write("squid_test", (void*) message, sizeof(message) / sizeof(char))) {
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
	
	switch (Squid_snapshot::global_squid->_read("squid_test", (void*) echo)) {
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
    
//     void squid_hash(char *me) 
//     {
// 	Squid_snapshot::global_squid->_hash(me);
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
