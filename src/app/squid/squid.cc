#include "squid.h"
#include "squidlib.h"
#include "util/bit_array.h"

#include <base/stdint.h>
#include <os/vfs.h>
#include <util/construct_at.h>
#include <util/misc_math.h>
#include <util/string.h>

namespace SquidSnapshot {

    SnapshotRoot::SnapshotRoot()
      : freeindex(0)
      , freemask()
    {
        freelist = (L1Dir*)SquidSnapshot::squidutils->_heap.alloc(
          sizeof(L1Dir) * ROOT_SIZE);

        freemask.set(0, ROOT_SIZE);

        Genode::Directory::Path path = to_path();
        SquidSnapshot::squidutils->createdir(path);

        if (!SquidSnapshot::squidutils->_vfs_env.root_dir().directory(
              path.string())) {

            Genode::error(SQUID_ERROR_FMT "couldn't create directory: ", path);
        }

        for (Genode::uint64_t i = 0; i < ROOT_SIZE; i++) {
            construct_at<L1Dir>(freelist + i, this, i);
        }
    }

    SnapshotRoot::~SnapshotRoot(void)
    {
        SquidSnapshot::squidutils->_heap.free(freelist, 0);
    }

    Genode::Directory::Path SnapshotRoot::to_path(void)
    {
        Genode::String<1024> path("/", SQUIDROOT, "/current");
        return path;
    }

    bool SnapshotRoot::is_full(void)
    {
        return !freemask.get(0, ROOT_SIZE);
    }

    L1Dir* SnapshotRoot::get_entry(void)
    {
        if (is_full())
            return nullptr;

        for (; !is_full(); freeindex = (freeindex + 1) % ROOT_SIZE) {
            if (!freelist[freeindex].is_full()) {
                if (!freemask.get(freeindex, 1))
                    freemask.set(freeindex, 1);

                return &freelist[freeindex];
            } else {
                try {
                    freemask.clear(freeindex, 1);
                } catch (Genode::Bit_array_base::Invalid_clear) {
                }
            }
        }

        return nullptr;
    }

    void SnapshotRoot::return_entry(uint64_t index)
    {
        freemask.set(index, 1);
    }

    SquidFileHash* SnapshotRoot::get_hash(void)
    {
        // INFO: Loop until you get a valid l1 or is_full() due to the freemask
        // not being updated in the edge case when we try to retreive the last
        // l2 directory in the current l1.
        //
        // This logic applies to the inner loop responsible for getting a valid
        // l2 directory.
        //
        // When we retrieve the SquidFileHash pointer we don't need a loop since
        // the freemask gets updated correctly in all cases
        while (!is_full()) {
            L1Dir* l1 = get_entry();
            if (l1 == nullptr)
                continue;

            while (!l1->is_full()) {
                L2Dir* l2 = l1->get_entry();
                if (l2 == nullptr)
                    continue;

                return l2->get_entry();
            }
        }

        return nullptr;
    }

    L1Dir::L1Dir(SnapshotRoot* parent, uint64_t l1)
      : freeindex(0)
      , freemask()
      , l1_dir(l1)
      , parent(parent)
    {
        freelist = (L2Dir*)SquidSnapshot::squidutils->_heap.alloc(
          sizeof(L2Dir) * L1_SIZE);

        freemask.set(0, L1_SIZE);

        Genode::Directory::Path path = to_path();
        SquidSnapshot::squidutils->createdir(path);

        for (uint64_t i = 0; i < L1_SIZE; i++) {
            construct_at<L2Dir>(freelist + i, this, l1_dir, i);
        }
    }

    L1Dir::~L1Dir(void)
    {
        SquidSnapshot::squidutils->_heap.free(freelist, 0);
        parent->return_entry(l1_dir);
    }

    Genode::Directory::Path L1Dir::to_path(void)
    {
        Genode::String<1024> path(parent->to_path(), "/", l1_dir);
        return path;
    }

    bool L1Dir::is_full(void)
    {
        return !freemask.get(0, L1_SIZE);
    }

    L2Dir* L1Dir::get_entry(void)
    {
        if (is_full()) {
            return nullptr;
        }

        for (; !is_full(); freeindex = (freeindex + 1) % L1_SIZE) {
            if (!freelist[freeindex].is_full()) {
                if (!freemask.get(freeindex, 1))
                    freemask.set(freeindex, 1);

                return &freelist[freeindex];
            } else {
                try {
                    freemask.clear(freeindex, 1);
                } catch (Genode::Bit_array_base::Invalid_clear) {
                }
            }
        }

        return nullptr;
    }

    void L1Dir::return_entry(uint64_t index)
    {
        freemask.set(index, 1);
    }

    L2Dir::L2Dir(L1Dir* parent, uint64_t l1, uint64_t l2)
      : freeindex(0)
      , freemask()
      , l1_dir(l1)
      , l2_dir(l2)
      , parent(parent)
    {
        this->freelist = (SquidFileHash*)SquidSnapshot::squidutils->_heap.alloc(
          sizeof(SquidFileHash) * L2_SIZE);

        freemask.set(0, L2_SIZE);

        Genode::Directory::Path path = to_path();
        SquidSnapshot::squidutils->createdir(path);

        for (uint64_t i = 0; i < L2_SIZE; i++) {
            construct_at<SquidFileHash>(freelist + i, this, l1_dir, l2_dir, i);
        }
    }

    L2Dir::~L2Dir(void)
    {
        SquidSnapshot::squidutils->_heap.free(freelist, 0);
        parent->return_entry(l2_dir);
    }

    Genode::Directory::Path L2Dir::to_path(void)
    {
        Genode::String<1024> path(parent->to_path(), "/", l2_dir);
        return path;
    }

    bool L2Dir::is_full(void)
    {
        return !freemask.get(0, L2_SIZE);
    }

    SquidFileHash* L2Dir::get_entry(void)
    {
        if (is_full())
            return nullptr;

        for (;; freeindex = (freeindex + 1) % L2_SIZE) {
            if (freemask.get(freeindex, 1)) {
                freemask.clear(freeindex, 1);

                return &freelist[freeindex];
            }
        }

        return nullptr;
    }

    void L2Dir::return_entry(uint64_t index)
    {
        freemask.set(index, 1);
    }

    SquidFileHash::SquidFileHash(L2Dir* parent,
                                 uint64_t l1,
                                 uint64_t l2,
                                 uint64_t file)
      : l1_dir(l1)
      , l2_dir(l2)
      , file_id(file)
      , parent(parent)
    {
    }

    Error SquidFileHash::write(void* payload, size_t size)
    {
        if (!is_valid)
            return Error::InvalidHash;

        try {
            New_file file(SquidSnapshot::squidutils->_root_dir, to_path());

            if (file.append((const char*)payload, size) !=
                New_file::Append_result::OK) {

                return Error::WriteFile;
            }

        } catch (New_file::Create_failed) {
            return Error::CreateFile;
        }

        return Error::None;
    }

    Error SquidFileHash::read(void* payload)
    {
        if (!is_valid)
            return Error::InvalidHash;

        Readonly_file file(SquidSnapshot::squidutils->_root_dir, to_path());
        Readonly_file::At at{ 0 };

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

    void SquidFileHash::return_entry(void)
    {
        parent->return_entry(file_id);
        is_valid = false;
    }

    Genode::Directory::Path SquidFileHash::to_path(void)
    {
        if (!is_valid)
            throw InvalidHash();

        Genode::String<1024> hash(parent->to_path(), "/", file_id);
        return hash;
    }

    void SquidUtils::createdir(const Genode::Directory::Path& path)
    {
        Vfs::Vfs_handle* handle = nullptr;
        auto res = squidutils->_vfs_env.root_dir().opendir(
          path.string(), true, &handle, _heap);

        if (res != Vfs::Directory_service::OPENDIR_ERR_NODE_ALREADY_EXISTS &&
            res != Vfs::Directory_service::OPENDIR_OK) {
            Genode::error("Couldn't open directory: ", path);

            if (res == Vfs::Directory_service::OPENDIR_ERR_PERMISSION_DENIED)
                Genode::error("reason: permission");

            throw Genode::Exception();
        }

        if (handle)
            handle->close();
    }

    Main::Main(SquidSnapshot::SquidUtils*)
    {
        construct_at<SquidSnapshot::SnapshotRoot>(&root_manager);
    }

    void Main::init_snapshot(void) 
    {
	Genode::String<256> path("/", SQUIDROOT, "/current");
	SquidSnapshot::squidutils->createdir(path);
    }

    void Main::finish_snapshot(void)
    {
        Genode::int64_t timestamp =
          SquidSnapshot::squidutils->_timer.curr_time()
            .trunc_to_plain_us()
            .value;

        Genode::String<1024> snapshot_timestamp("/", SQUIDROOT, "/", timestamp);
        Genode::String<1024> snapshot_current("/", SQUIDROOT, "/current");

        if (SquidSnapshot::squidutils->_vfs_env.root_dir().rename(
              snapshot_current.string(), snapshot_timestamp.string()) ==
            Vfs::Directory_service::RENAME_ERR_NO_ENTRY) {
            Genode::error("rename no good!");
        }
    }

    Error Main::test(void)
    {
        char message[] = "payload";

        SquidFileHash* hash = global_squid->root_manager.get_hash();
        if (hash == nullptr)
            return Error::OutOfHashes;

        switch (hash->write((void*)message, sizeof(message) / sizeof(char))) {

            case Error::CreateFile:
                return Error::CreateFile;

            case Error::WriteFile:
                return Error::WriteFile;

            default:
                break;
        }

        char* echo =
          (char*)SquidSnapshot::squidutils->_heap.alloc(sizeof(char) * 20);

        switch (hash->read((void*)echo)) {

            case Error::ReadFile:
                return Error::ReadFile;

            default:
                break;
        }

        char* i = echo;
        char* j = message;

        while (*i != 0 && *j != 0) {
            if (*i != *j)
                return Error::CorruptedFile;

            i++;
            j++;
        }

        if (*i != 0 || *j != 0)
            return Error::CorruptedFile;

        try {
            hash->return_entry();
        } catch (...) {
            return Error::DeleteFile;
        }

        return Error::None;
    }
}; // namespace SquidSnapshot

#ifdef __cplusplus

extern "C"
{

    enum SquidError squid_hash(void** hash)
    {
        SquidSnapshot::SquidFileHash* squid_generated_hash =
          SquidSnapshot::global_squid->root_manager.get_hash();

        if (squid_generated_hash == nullptr)
            return SQUID_FULL;

        *hash = (void*)squid_generated_hash;
        return SQUID_NONE;
    }

    enum SquidError squid_write(void* hash,
                                void* payload,
                                unsigned long long size)
    {
        SquidSnapshot::SquidFileHash* squid_file =
          (SquidSnapshot::SquidFileHash*)hash;

        switch (squid_file->write(payload, size)) {
            case SquidSnapshot::Error::CreateFile:
                return SQUID_CREATE;

            case SquidSnapshot::Error::WriteFile:
                return SQUID_WRITE;

            default:
                return SQUID_NONE;
        }
    }

    enum SquidError squid_read(void* hash, void* payload)
    {
        SquidSnapshot::SquidFileHash* squid_file =
          (SquidSnapshot::SquidFileHash*)hash;

        switch (squid_file->read(payload)) {
            case SquidSnapshot::Error::ReadFile:
                return SQUID_READ;

            default:
                return SQUID_NONE;
        }
    }

    enum SquidError squid_delete(void* hash)
    {
        SquidSnapshot::SquidFileHash* file =
          (SquidSnapshot::SquidFileHash*)hash;

        try {
            file->return_entry();
        } catch (...) {
            return SQUID_DELETE;
        }

        return SQUID_NONE;
    }

    enum SquidError squid_test(void)
    {
        switch (SquidSnapshot::global_squid->test()) {
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

    void squid_init_snapshot(void) 
    {
	SquidSnapshot::global_squid->init_snapshot();
    }

    void squid_finish_snapshot(void) 
    {
	SquidSnapshot::global_squid->finish_snapshot();
    }
}

#endif // __cplusplus
