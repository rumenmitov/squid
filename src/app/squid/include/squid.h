/**
 * @Author Rumen Mitov
 * @Date 2024-09-07

 squid.h provides an API to snapshot data to the disk.
 It is organized as a trie with an Upper Directory (L1), and a
 Lower Directory (L2).

 L1         - determined by the first two chars of the hash.
 L2         - determined by the next pair of chars of the hash.
 Squid File - determined by remaining chars in hash.

 All snapshots are stored in the root directory SquidSnapshot::SQUIDROOT,
 which will be referred to as <squidroot> from now on.

 An ongoing snapshot is contained within the <squidroot>/current directory.
 Completed snapshots are stored within /<squidroot>/<timestamp>, where <timestamp>
 is a unix timestamp of when the snapshot was completed.
*/

#ifndef __SQUID_H
#define __SQUID_H

#ifdef __cplusplus

#include <base/component.h>
#include <base/sleep.h>
#include <base/attached_rom_dataspace.h>
#include <base/heap.h>
#include <util/bit_array.h>
#include <os/vfs.h>
#include <vfs/file_system_factory.h>
#include <base/buffered_output.h>

#define BITS_PER_WORD sizeof(addr_t) * 8UL
#define WORD_ALIGN(_BITS) (BITS_PER_WORD) - (_BITS % (BITS_PER_WORD)) + _BITS

namespace SquidSnapshot {
    using namespace Genode;

    /**
     * @brief Error types associated with Squid Cache.
     */
    enum Error {
	OutOfHashes,
	WriteFile,
	ReadFile,
	CreateFile,
	CorruptedFile,
	None
    };

    /**
     * @brief The root directory containing all snapshots.
     */
    const static char SQUIDROOT[] = "squid-root";
    

    /**
     * @brief Amount of entries in each level of the snapshot hierarchy.
     */
    static const unsigned int ROOT_SIZE = WORD_ALIGN(16);
    static const unsigned int L1_SIZE 	= WORD_ALIGN(256);
    static const unsigned int L2_SIZE 	= WORD_ALIGN(1000);


    struct Main;
    class SnapshotRoot;
    class L1Dir;
    class L2Dir;
    class SquidFileHash;

    /**
     * @brief Manages L1 directories in the snapshot root.
     */
    class SnapshotRoot
    {
    protected:
	unsigned int capacity;
        L1Dir *freelist = nullptr;
	unsigned int freeindex;
	Genode::Bit_array<ROOT_SIZE> freemask;

	SnapshotRoot(const SnapshotRoot&) = delete;
	SnapshotRoot& operator=(const SnapshotRoot&) = delete;

    public:
	SnapshotRoot();
	~SnapshotRoot(void);

	void init(unsigned int capacity = ROOT_SIZE);

	Genode::Directory::Path to_path(void);
	bool is_full(void);

	L1Dir* get_entry(void);
	void return_entry(unsigned int);

	SquidFileHash* get_hash(void);
    };


    /**
     * @brief Manages L2 directories in an L1 directory instance.
     */
    class L1Dir 
    {
    private:
	unsigned int capacity;
        L2Dir *freelist = nullptr;
	unsigned int freeindex;
	Genode::Bit_array<L1_SIZE> freemask;

	unsigned int l1_dir;

        SnapshotRoot *parent;

	L1Dir(const L1Dir&) = delete;
	L1Dir &operator= (const L1Dir &) = delete;


    public:
	L1Dir();
	~L1Dir(void);

	void init(SnapshotRoot*, unsigned int, unsigned int capacity = L1_SIZE);

	Genode::Directory::Path to_path(void);
	bool is_full(void);

	L2Dir* get_entry(void);
	void return_entry(unsigned int);
    };


    /**
     * @brief Manages free hashes in an L2 directory instance.
     */
    class L2Dir 
    {
    private:
	unsigned int capacity;
	SquidFileHash *freelist = nullptr;
	unsigned int freecount;

	unsigned int l1_dir;
	unsigned int l2_dir;

        L1Dir *parent;

	L2Dir(const L2Dir&) = delete;
        L2Dir &operator= (const L2Dir &) = delete;

    public:
	L2Dir();
	~L2Dir(void);

	void init(L1Dir*, unsigned int l1, unsigned int l2, unsigned int capacity = L2_SIZE);

	Genode::Directory::Path to_path(void);
	bool is_full(void);

	SquidFileHash* get_entry(void);
	void return_entry(void);
    };


    /**
     * @brief Represents squid hash of a file. Comprised of L1, L2 directories and the file id.
     */
    struct SquidFileHash 
    {
	unsigned int l1_dir;
	unsigned int l2_dir;
	unsigned int file_id;

	L2Dir *parent;

        SquidFileHash(L2Dir *, unsigned int, unsigned int, unsigned int);
	SquidFileHash(const SquidFileHash&);

        ~SquidFileHash(void);

	SquidFileHash& operator=(const SquidFileHash&);
	
	Genode::Directory::Path to_path(void);
    };

    
    typedef Directory::Path Path;


    struct SquidUtils
    {
	Env &_env;
        SquidUtils(Env &env) : _env(env) 
	{};
	
	Heap _heap{_env.ram(), _env.rm()};

        Attached_rom_dataspace _config{_env, "config"};

        Vfs::Global_file_system_factory _fs_factory{_heap};

        Vfs::Simple_env _vfs_env{_env, _heap, _config.xml().sub_node("vfs")};
	
	Directory _root_dir { _vfs_env };
    };


    struct Main
    {
	/* INFO
	   The constructor should be called only AFTER SquidUtils
	   has been initialized.
	*/
	Main() = delete;
        Main(SquidSnapshot::SquidUtils*) {};
	
	
	/**
	 * @brief Responsible for managing file structure of snapshot.
	 */
	SnapshotRoot root_manager {};


	/**
         * @brief Writes payload to file (creates one if it does not exist).
         */	
	enum Error _write(Path const &path, void *payload, size_t size);

	
       /**
        * @brief Reads from squid file into payload buffer.
	*/
	enum Error _read(Path const &path, void *payload);


	/**
	 * @brief Unit test.
	 */
	enum Error _test(void);

    };

    extern SquidUtils *squidutils;
    extern Main *global_squid;
};


#endif // __cplusplus

#endif // __SQUID_H
