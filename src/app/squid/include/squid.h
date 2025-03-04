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
 Completed snapshots are stored within /<squidroot>/<timestamp>, where
 <timestamp> is a unix timestamp of when the snapshot was completed.

 The parent structure (SnapshotRoot for L1, L1 for L2, and L2 for
 Squid File) keeps track of its free children via a bit array. If the
 bit is 1, the child is available and vice versa.
*/

#ifndef __SQUID_H
#define __SQUID_H

#ifdef __cplusplus

#include <base/attached_rom_dataspace.h>
#include <base/buffered_output.h>
#include <base/component.h>
#include <base/heap.h>
#include <base/sleep.h>
#include <os/vfs.h>
#include <timer_session/connection.h>
#include <util/bit_array.h>
#include <vfs/file_system_factory.h>

#define BITS_PER_WORD sizeof(addr_t) * 8UL
#define WORD_ALIGN(_BITS) (BITS_PER_WORD) - (_BITS % (BITS_PER_WORD)) + _BITS

namespace SquidSnapshot {
    using namespace Genode;

    /**
     * @brief Error types associated with Squid Cache.
     */
    enum Error
    {
        OutOfHashes,
	InvalidHash,
        WriteFile,
        ReadFile,
        CreateFile,
        CorruptedFile,
        DeleteFile,
        None
    };

    /**
     * @brief The root directory containing all snapshots.
     */
    const static char SQUIDROOT[] = "squid-root";

    /**
     * @brief Amount of entries in each level of the snapshot hierarchy.
     */
    // BUG: Program halts for large values of ROOT_SIZE * L1_SIZE * L2_SIZE.
    static const uint64_t ROOT_SIZE = 5;
    static const uint64_t __ROOT_SIZE = WORD_ALIGN(ROOT_SIZE);

    static const uint64_t L1_SIZE = 5;
    static const uint64_t __L1_SIZE = WORD_ALIGN(L1_SIZE);

    static const uint64_t L2_SIZE = 5;
    static const uint64_t __L2_SIZE = WORD_ALIGN(L2_SIZE);

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
      private:
        L1Dir* freelist = nullptr;
        uint64_t freeindex;
        Genode::Bit_array<__ROOT_SIZE> freemask;

        SnapshotRoot(const SnapshotRoot&) = delete;
        SnapshotRoot& operator=(const SnapshotRoot&) = delete;

      public:
        SnapshotRoot();
        ~SnapshotRoot(void);

        Genode::Directory::Path to_path(void);
        bool is_full(void);

        L1Dir* get_entry(void);
        void return_entry(uint64_t);

        SquidFileHash* get_hash(void);
    };

    /**
     * @brief Manages L2 directories in an L1 directory instance.
     */
    class L1Dir
    {
      private:
        L2Dir* freelist = nullptr;
        uint64_t freeindex;
        Genode::Bit_array<__L1_SIZE> freemask;

        uint64_t l1_dir;

        SnapshotRoot* parent;

        L1Dir(const L1Dir&) = delete;
        L1Dir& operator=(const L1Dir&) = delete;

      public:
        L1Dir(SnapshotRoot*, uint64_t);
        ~L1Dir(void);

        Genode::Directory::Path to_path(void);
        bool is_full(void);

        L2Dir* get_entry(void);
        void return_entry(uint64_t);
    };

    /**
     * @brief Manages free hashes in an L2 directory instance.
     */
    class L2Dir
    {
      private:
        SquidFileHash* freelist = nullptr;
        uint64_t freeindex;
        Genode::Bit_array<__L2_SIZE> freemask;

        uint64_t l1_dir;
        uint64_t l2_dir;

        L1Dir* parent;

        L2Dir(const L2Dir&) = delete;
        L2Dir& operator=(const L2Dir&) = delete;

      public:
        L2Dir(L1Dir*, uint64_t l1, uint64_t l2);
        ~L2Dir(void);

        Genode::Directory::Path to_path(void);
        bool is_full(void);

        SquidFileHash* get_entry(void);
        void return_entry(uint64_t);
    };

    /**
     * @brief Represents squid hash of a file. Comprised of L1, L2 directories
     * and the file id.
     * @exception InvalidHash is thrown for any operations on the
     * object, if it has been previously returned to the parent L2.
     */
    class SquidFileHash
    {
      private:
        uint64_t l1_dir;
        uint64_t l2_dir;
        uint64_t file_id;

        L2Dir* parent;

        SquidFileHash(const SquidFileHash&) = delete;
        SquidFileHash& operator=(const SquidFileHash&) = delete;

      public:
        class InvalidHash : public Exception
        {};

        bool is_valid = true;

        SquidFileHash(L2Dir*, uint64_t, uint64_t, uint64_t);

        Genode::Directory::Path to_path(void);

        /**
         * @brief Writes payload to file (creates one if it does not exist).
         */
        enum Error write(void* payload, size_t size);

        /**
         * @brief Reads from squid file into payload buffer.
         */
        enum Error read(void* payload);

        /**
         * @brief Returns hash back to L2 parent, and invalidates this object.
         */
        void return_entry(void);
    };

    typedef Directory::Path Path;

    struct SquidUtils
    {
        Env& _env;
        SquidUtils(Env& env)
          : _env(env){};

        Heap _heap{ _env.ram(), _env.rm() };
        Attached_rom_dataspace _config{ _env, "config" };

        Vfs::Simple_env _vfs_env{ _env, _heap, _config.xml().sub_node("vfs") };
        Root_directory _root_dir{ _env, _heap, _config.xml().sub_node("vfs") };

        Genode::Entrypoint _ep_timer{ _env,
                                      sizeof(Genode::addr_t) * 2048,
                                      "entrypoint_timer",
                                      Genode::Affinity::Location() };

        Timer::Connection _timer{ _env, _ep_timer, "squid_timer" };

        // TODO proper error handling
        void createdir(const Genode::Directory::Path& path);
    };

    class Main
    {
      private:
        Main() = delete;
        Main(const Main&) = delete;
        Main& operator=(const Main&) = delete;

      public:
        /* INFO
           The constructor should be called only AFTER SquidUtils
           has been initialized.
        */
        Main(SquidSnapshot::SquidUtils*);
        void finish();

        /**
         * @brief Responsible for managing file structure of snapshot.
         */
        SnapshotRoot root_manager{};

        /**
         * @brief Unit test.
         */
        enum Error test(void);
    };

    extern SquidUtils* squidutils;
    extern Main* global_squid;
};

#endif // __cplusplus

#endif // __SQUID_H
