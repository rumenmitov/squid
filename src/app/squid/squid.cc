#include "squid.h"
#include "squidlib.h"

#include <os/vfs.h>
#include <util/string.h>
#include <util/misc_math.h>
#include <format/snprintf.h>
#include <base/stdint.h>


// static void* squid_malloc(Genode::size_t size) 
// {
//     Genode::size_t total_size = sizeof(Genode::size_t) + size;

//     auto alloc_res = SquidSnapshot::squidutils->_heap.try_alloc(total_size);

//     if (!alloc_res.ok()){
//         alloc_res.with_error([](Genode::Allocator::Alloc_error err){ Genode::error(err); });
//         return 0;
//     }

//     void* original_addr = nullptr;

//     alloc_res.with_result(
//         [&](void* addr){ original_addr = addr; }, 
//         [&](Genode::Allocator::Alloc_error err){ Genode::error(err); }
//     );

//     // just to ensure it is safe
//     if (original_addr == nullptr){
// 	Genode::error("malloc: addr = nullptr!!!");
//         return 0;
//     }

//     // Writing the size
//     *((Genode::size_t *)original_addr) = size;

//     Genode::size_t *adjusted_addr = ((Genode::size_t *)original_addr) + 1;

//     return (void *)adjusted_addr;
// }


static void squid_free(void* addr) 
{
    if (addr == nullptr) return;

    // Genode::size_t *adjusted_addr = (Genode::size_t *)addr;
    // void *original_addr = (void *)(adjusted_addr - 1);

    // Genode::size_t size = *((Genode::size_t *)original_addr);

    SquidSnapshot::squidutils->_heap.free(addr, 0);
    addr = nullptr;
}


namespace SquidSnapshot {

    SnapshotRoot::SnapshotRoot(unsigned int capacity)
	: capacity(capacity), freecount(capacity)
    {

	freelist = (L1Dir *) SquidSnapshot::squidutils->_heap.alloc(sizeof(L1Dir) * capacity);

        Genode::Directory::Path path = to_path();

	SquidSnapshot::squidutils->_root_dir.create_sub_directory(path);
	if (!SquidSnapshot::squidutils->_root_dir.directory_exists(path)) {
	    Genode::error(SQUID_ERROR_FMT "couldn't create directory: ", path);
        }

	for (unsigned int i = 0; i < capacity; i++) {
	    freelist[i] = L1Dir(this, i, L1_SIZE);
        }
    }

    SnapshotRoot::SnapshotRoot(const SnapshotRoot &other)
	: capacity(other.capacity), freecount(other.freecount)
    {
	freelist = (L1Dir*) SquidSnapshot::squidutils->_heap.alloc(sizeof(L1Dir) * capacity);

	Genode::Directory::Path path = to_path();

        SquidSnapshot::squidutils->_root_dir.create_sub_directory(path);
	if (!SquidSnapshot::squidutils->_root_dir.directory_exists(path)) {
	    Genode::error(SQUID_ERROR_FMT "couldn't create directory: ", path);
	}

	for (unsigned int i = 0; i < capacity; i++) {
	    freelist[i] = L1Dir(this, i, L1_SIZE);
	}
    }


    SnapshotRoot::~SnapshotRoot(void)
    {
	squid_free(freelist);
    }


    SnapshotRoot& SnapshotRoot::operator=(const SnapshotRoot &other)
    {
	capacity = other.capacity;
	freecount = other.freecount;
	
	freelist = (L1Dir*) SquidSnapshot::squidutils->_heap.alloc(sizeof(L1Dir) * capacity);

	Genode::Directory::Path path = to_path();

	SquidSnapshot::squidutils->_root_dir.create_sub_directory(path);
	if (!SquidSnapshot::squidutils->_root_dir.directory_exists(path)) {
	    Genode::error(SQUID_ERROR_FMT "couldn't create directory: ", path);
	}

	for (unsigned int i = 0; i < capacity; i++) {
	    freelist[i] = L1Dir(this, i, L1_SIZE);
	}

	return *this;
    }
    

    Genode::Directory::Path SnapshotRoot::to_path(void) 
    {
	char path[strlen("//current") + strlen(SQUIDROOT)];
	Format::snprintf(path, 64, "/%s/current", SQUIDROOT);
	
	return path;
    }


    bool SnapshotRoot::is_full(void) 
    {
	freecount = L1_SIZE;
	
	for (unsigned int i = 0; i < capacity; i++) {
	    if (freelist[i].is_full()) {
		freecount--;
	    }
	}
	
	return freecount == 0;
    }
    
    
    L1Dir* SnapshotRoot::get_entry(void)
    {
	if (is_full()) return nullptr;

	for (unsigned i = 0; i < capacity; i++) {
	    if (!freelist[i].is_full()) {
		freecount--;
		return &freelist[i];
	    }
	}

	Genode::error(SQUID_ERROR_FMT "This state should not be reacheable!");
	return nullptr;
    }


    void SnapshotRoot::return_entry(void) 
    {
	if (freecount == capacity) return;
	freecount++;
    }


    SquidFileHash* SnapshotRoot::get_hash(void) 
    {
	L1Dir *l1 = get_entry();
	if (l1 == nullptr) return nullptr;

	L2Dir *l2 = l1->get_entry();
	if (l2 == nullptr) return nullptr;

	return l2->get_entry();
    }
    


    L1Dir::L1Dir(SnapshotRoot *parent, unsigned int l1, unsigned int capacity)
	: capacity(capacity), freecount(capacity), l1_dir(l1), parent(parent)
    {
	freelist = (L2Dir*) SquidSnapshot::squidutils->_heap.alloc(sizeof(L2Dir) * capacity);

	Genode::Directory::Path path = to_path();

	SquidSnapshot::squidutils->_root_dir.create_sub_directory(path);
	if (!SquidSnapshot::squidutils->_root_dir.directory_exists(path)) {
	    Genode::error(SQUID_ERROR_FMT "couldn't create directory: ", path);
	}

	for (unsigned int i = 0; i < capacity; i++) {
	    freelist[i] = L2Dir(this, l1_dir, i, L2_SIZE);
        }
    }


    L1Dir::L1Dir(const L1Dir &other)
	: capacity(other.capacity), freecount(other.freecount), l1_dir(other.l1_dir), parent(other.parent)
    {
	freelist = (L2Dir*) SquidSnapshot::squidutils->_heap.alloc(sizeof(L2Dir) * capacity);

	Genode::Directory::Path path = to_path();

	SquidSnapshot::squidutils->_root_dir.create_sub_directory(path);
	if (!SquidSnapshot::squidutils->_root_dir.directory_exists(path)) {
	    Genode::error(SQUID_ERROR_FMT "couldn't create directory: ", path);
	}

	for (unsigned int i = 0; i < capacity; i++) {
	    freelist[i] = L2Dir(this, l1_dir, i, L2_SIZE);
	}
    }
    

    L1Dir::~L1Dir(void)
    {
	squid_free(freelist);
	parent->return_entry();
    }


    L1Dir& L1Dir::operator=(const L1Dir &other)
    {
	parent = other.parent;
	l1_dir = other.l1_dir;
	capacity = other.capacity;
	freecount = other.freecount;
	
	freelist = (L2Dir*) SquidSnapshot::squidutils->_heap.alloc(sizeof(L2Dir) * capacity);

	Genode::Directory::Path path = to_path();

	SquidSnapshot::squidutils->_root_dir.create_sub_directory(path);
	if (!SquidSnapshot::squidutils->_root_dir.directory_exists(path)) {
	    Genode::error(SQUID_ERROR_FMT "couldn't create directory: ", path);
	}

	for (unsigned int i = 0; i < capacity; i++) {
	    freelist[i] = L2Dir(this, l1_dir, i, L2_SIZE);
        }
	
	return *this;
    }
    

    Genode::Directory::Path L1Dir::to_path(void) 
    {

	Genode::size_t digits = (log2(ROOT_SIZE) / log2(16)) + 1;
	Genode::size_t total_chars = strlen("//current/") + strlen(SQUIDROOT) + digits;
      
	char path[total_chars];
	Format::snprintf(path, total_chars, "/%s/current/%x", SQUIDROOT, l1_dir);

	return path;
    }


    bool L1Dir::is_full(void) 
    {
	freecount = L1_SIZE;
	
	for (unsigned int i = 0; i < capacity; i++) {
	    if (freelist[i].is_full()) {
		freecount--;
	    }
	}
	
	return freecount == 0;
    }

    
    L2Dir* L1Dir::get_entry(void)
    {
	if (is_full()) return nullptr;

	for (unsigned i = 0; i < capacity; i++) {
	    if (!freelist[i].is_full()) {
		return &freelist[i];
	    }
	}
	Genode::error(SQUID_ERROR_FMT "This state should not be reacheable!");
	return nullptr;
    }


    void L1Dir::return_entry(void) 
    {
	if (freecount == capacity) return;
	freecount++;
    }


    L2Dir::L2Dir(L1Dir *parent, unsigned int l1, unsigned int l2, unsigned int capacity)
	: capacity(capacity), freecount(capacity), l1_dir(l1), l2_dir(l2), parent(parent)
    {

	freelist = (SquidFileHash *) SquidSnapshot::squidutils->_heap.alloc(sizeof(SquidFileHash) * capacity);

	Genode::Directory::Path path = to_path();

	SquidSnapshot::squidutils->_root_dir.create_sub_directory(path);
	if (!SquidSnapshot::squidutils->_root_dir.directory_exists(path)) {
	    Genode::error(SQUID_ERROR_FMT "couldn't create directory: ", path);
	}

	for (unsigned int i = 0; i < capacity; i++) {
	    freelist[i] = SquidFileHash(this, l1_dir, l2_dir, i);
        }
    }


    L2Dir::L2Dir(const L2Dir &other)
	: capacity(other.capacity), freecount(other.freecount), l1_dir(other.l1_dir), l2_dir(other.l2_dir), parent(other.parent)
    {
	freelist = (SquidFileHash *) SquidSnapshot::squidutils->_heap.alloc(sizeof(SquidFileHash) * capacity);

	Genode::Directory::Path path = to_path();

	SquidSnapshot::squidutils->_root_dir.create_sub_directory(path);
	if (!SquidSnapshot::squidutils->_root_dir.directory_exists(path)) {
	    Genode::error(SQUID_ERROR_FMT "couldn't create directory: ", path);
	}

	for (unsigned int i = 0; i < capacity; i++) {
	    freelist[i] = SquidFileHash(this, l1_dir, l2_dir, i);
        }
    }


    L2Dir::~L2Dir(void)
    {
	/* INFO
	   Used SquidFileHashes will be deleted from disk
	   when the corresponding SquidFileHash's destructor is called.
	*/
	squid_free(freelist);
	parent->return_entry();
    }


    L2Dir& L2Dir::operator=(const L2Dir &other) 
    {
	parent = other.parent;
	l1_dir = other.l1_dir;
	l2_dir = other.l2_dir;
	capacity = other.capacity;
	freecount = other.freecount;
	
	freelist = (SquidFileHash *) SquidSnapshot::squidutils->_heap.alloc(sizeof(SquidFileHash) * capacity);

	Genode::Directory::Path path = to_path();

	SquidSnapshot::squidutils->_root_dir.create_sub_directory(path);
	if (!SquidSnapshot::squidutils->_root_dir.directory_exists(path)) {
	    Genode::error(SQUID_ERROR_FMT "couldn't create directory: ", path);
	}

	for (unsigned int i = 0; i < capacity; i++) {
	    freelist[i] = SquidFileHash(this, l1_dir, l2_dir, i);
        }
	
	return *this;
    }
    

    Genode::Directory::Path L2Dir::to_path(void) 
    {
	Genode::size_t root_digits = (log2(ROOT_SIZE) / log2(16)) + 1;
      	Genode::size_t l1_digits = (log2(L1_SIZE) / log2(16)) + 1;
	Genode::size_t total_chars = strlen("//current//") + strlen(SQUIDROOT) + root_digits + l1_digits;

	char path[total_chars];
	Format::snprintf(path, total_chars, "/%s/current/%x/%x", SQUIDROOT, l1_dir, l2_dir);

	return path;
    }


    bool L2Dir::is_full(void) 
    {
	return freecount == 0;
    }

    
    SquidFileHash* L2Dir::get_entry(void)
    {
	if (is_full()) {
	    Genode::log("l2 full");
	    return nullptr;
	}
      
		

	// TODO implement as ring buffer with memory
	for (unsigned int i = 0; i < capacity; i++) {
	    Genode::Directory::Path hash = freelist[i].to_path();
	    
	    if (!SquidSnapshot::squidutils->_root_dir.file_exists(hash)) {
		freecount--;

                try {
                  New_file file(SquidSnapshot::squidutils->_root_dir, hash);

		  if ( file.append("", 0) != New_file::Append_result::OK )
		      throw Error::WriteFile;

		  Genode::log("acquired ", hash);

		  
		} catch (New_file::Create_failed) {
		    throw Error::CreateFile;
                }

		return &freelist[i];
	    }
	}

	Genode::error(SQUID_ERROR_FMT "This state should not be reacheable!");
	return nullptr;
    }


    void L2Dir::return_entry(void) 
    {
	if (freecount == capacity) return;
	freecount++;
    }


    SquidFileHash::SquidFileHash(L2Dir *parent, unsigned int l1, unsigned int l2,
                                 unsigned int file)
        : l1_dir(l1), l2_dir(l2), file_id(file), parent(parent)
    {}


    SquidFileHash::SquidFileHash(const SquidFileHash &other)
        : l1_dir(other.l1_dir), l2_dir(other.l2_dir), file_id(other.file_id),
          parent(other.parent)
    {}
    
    
    SquidFileHash::~SquidFileHash(void) 
    {
	parent->return_entry();
    }


    SquidFileHash& SquidFileHash::operator=(const SquidFileHash &other)
    {
      l1_dir = other.l1_dir;
      l2_dir = other.l2_dir;
      file_id = other.file_id;

      parent = other.parent;

      return *this;
    }
    


    Genode::Directory::Path SquidFileHash::to_path(void) 
    {
       	Genode::size_t root_digits = (log2(ROOT_SIZE) / log2(16)) + 1;
        Genode::size_t l1_digits = (log2(L1_SIZE) / log2(16)) + 1;
	Genode::size_t l2_digits = (log2(L2_SIZE) / log2(16)) + 1;
	Genode::size_t total_chars = strlen("//current///") + strlen(SQUIDROOT) + root_digits + l1_digits + l2_digits;

	char hash[total_chars];

        Format::snprintf(hash, total_chars, "/%s/current/%x/%x/%x",
			 SquidSnapshot::SQUIDROOT,
			 l1_dir,
			 l2_dir,
			 file_id);

	return hash;
    }    
    

    Error Main::_write(Path const &path, void *payload, size_t size) 
    {
	try {
	    New_file file(SquidSnapshot::squidutils->_root_dir, path);

	    if ( file.append((const char *) payload, size) != New_file::Append_result::OK )
		return Error::WriteFile;
		  
	} catch (New_file::Create_failed) {
	    return Error::CreateFile;
	}

	return Error::None;
    }

	  
    Error Main::_read(Path const &path, void *payload) 
    {
	Readonly_file file(SquidSnapshot::squidutils->_root_dir, path);
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


    Error Main::_test(void) 
    {
	char message[] = "payload";

	SquidFileHash *hash = global_squid->root_manager.get_hash();
	if (hash == nullptr) return Error::OutOfHashes;

	switch (Main::_write(hash->to_path(), (void*) message, sizeof(message) / sizeof(char))) {
	case Error::CreateFile:
	    return Error::CreateFile;

	case Error::WriteFile:
	    return Error::WriteFile;

	default:
	    break;
	}


        char *echo = (char*) SquidSnapshot::squidutils->_heap.alloc(sizeof(char) * 20);
	
	switch (SquidSnapshot::global_squid->_read(hash->to_path(), (void*) echo)) {
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


#ifdef __cplusplus

extern "C" {

    void squid_hash(void **hash) 
    {
	// #warning should handle fail case (no more hashes)
	SquidSnapshot::SquidFileHash *squid_generated_hash = SquidSnapshot::global_squid->root_manager.get_hash();
	*hash = (void*) squid_generated_hash;
    }


    enum SquidError squid_write(void *hash, void *payload, unsigned long long size) 
    {
	auto path = ((SquidSnapshot::SquidFileHash*) hash)->to_path();
	
	switch (SquidSnapshot::global_squid->_write(path, payload, size)) {
	case SquidSnapshot::Error::CreateFile:
	    return SQUID_CREATE;

	case SquidSnapshot::Error::WriteFile:
	    return SQUID_WRITE;

	default:
	    return SQUID_NONE;
	}
    }

    
    enum SquidError squid_read(void *hash, void *payload) 
    {
	auto path = ((SquidSnapshot::SquidFileHash*) hash)->to_path();
	
	switch ( SquidSnapshot::global_squid->_read(path, payload) ) {
	case SquidSnapshot::Error::ReadFile:
	    return SQUID_READ;

	default:
	    return SQUID_NONE;
	}
    }
    
    enum SquidError squid_delete(void *hash) 
    {
	SquidSnapshot::SquidFileHash* file = (SquidSnapshot::SquidFileHash*) hash;

	try {
	    delete file;
	} catch (...) {
	    return SQUID_DELETE;
	}
	
	return SQUID_NONE;
    }

    enum SquidError squid_test(void) 
    {
	switch (SquidSnapshot::global_squid->_test()) {
	case SquidSnapshot::Error::CreateFile:
	    return SQUID_CREATE;

	case SquidSnapshot::Error::WriteFile:
	    return SQUID_WRITE;

	case SquidSnapshot::Error::ReadFile:
	    return SQUID_READ;

	case SquidSnapshot::Error::CorruptedFile:
	    return SQUID_CORRUPTED;

	default:
	    return SQUID_NONE;
	}
    }
}

#endif // __cplusplus
