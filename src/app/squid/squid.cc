#include "squid.h"
#include "base/exception.h"
#include "squidlib.h"
#include "vfs/directory_service.h"
#include "vfs/vfs_handle.h"

#include <base/stdint.h>
#include <format/snprintf.h>
#include <os/vfs.h>
#include <util/construct_at.h>
#include <util/misc_math.h>
#include <util/string.h>

namespace SquidSnapshot {

    SnapshotRoot::SnapshotRoot()
      : freeindex(0)
      , freemask()
    {
        this->freelist = (L1Dir*)SquidSnapshot::squidutils->_heap.alloc(
          sizeof(L1Dir) * ROOT_SIZE);

        this->freemask.set(0, ROOT_SIZE);

        Genode::Directory::Path path = to_path();
        SquidSnapshot::squidutils->createdir(path);

        if (!SquidSnapshot::squidutils->_root_dir.directory_exists(path)) {
            Genode::error(SQUID_ERROR_FMT "couldn't create directory: ", path);
        }

        for (unsigned int i = 0; i < ROOT_SIZE; i++) {
            construct_at<L1Dir>(&freelist[i], this, i);
        }
    }

    SnapshotRoot::~SnapshotRoot(void)
    {
        SquidSnapshot::squidutils->_heap.free(freelist, 0);
    }

    Genode::Directory::Path SnapshotRoot::to_path(void)
    {
        char path[strlen("//current") + strlen(SQUIDROOT)];
        Format::snprintf(path, 64, "/%s/current", SQUIDROOT);

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

        for (;; freeindex = (freeindex + 1) % ROOT_SIZE) {
            if (!freelist[freeindex].is_full()) {
                if (!freemask.get(freeindex, 1))
                    freemask.set(freeindex, 1);

                return &freelist[freeindex];
            } else {
                freemask.clear(freeindex, 1);
            }
        }

        Genode::error(SQUID_ERROR_FMT "This state should not be reacheable!");
        return nullptr;
    }

    void SnapshotRoot::return_entry(unsigned int index)
    {
        freemask.set(index, 1);
    }

    SquidFileHash* SnapshotRoot::get_hash(void)
    {
        L1Dir* l1 = get_entry();

        if (l1 == nullptr)
            return nullptr;

        L2Dir* l2 = l1->get_entry();

        if (l2 == nullptr)
            return nullptr;

        return l2->get_entry();
    }

    L1Dir::L1Dir(SnapshotRoot* parent, unsigned int l1)
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

        if (!SquidSnapshot::squidutils->_root_dir.directory_exists(path)) {
            Genode::error(SQUID_ERROR_FMT "couldn't create directory: ", path);
        }

        for (unsigned int i = 0; i < L1_SIZE; i++) {
            construct_at<L2Dir>(&freelist[i], this, l1_dir, i);
        }
    }

    L1Dir::~L1Dir(void)
    {
        SquidSnapshot::squidutils->_heap.free(freelist, 0);
        parent->return_entry(l1_dir);
    }

    Genode::Directory::Path L1Dir::to_path(void)
    {
        Genode::size_t digits = (log2(ROOT_SIZE) / log2(16)) + 1;

        Genode::size_t total_chars =
          strlen("//current/") + strlen(SQUIDROOT) + digits;

        char path[total_chars];

        Format::snprintf(
          path, total_chars, "/%s/current/%x", SQUIDROOT, l1_dir);

        return path;
    }

    bool L1Dir::is_full(void)
    {
        return !freemask.get(0, L1_SIZE);
    }

    L2Dir* L1Dir::get_entry(void)
    {
        if (is_full())
            return nullptr;

        for (;; freeindex = (freeindex + 1) % L1_SIZE) {
            if (!freelist[freeindex].is_full()) {
                if (!freemask.get(freeindex, 1))
                    freemask.set(freeindex, 1);

                return &freelist[freeindex];
            } else {
                freemask.clear(freeindex, 1);
            }
        }

        Genode::error(SQUID_ERROR_FMT "This state should not be reacheable!");
        return nullptr;
    }

    void L1Dir::return_entry(unsigned int index)
    {
        freemask.set(index, 1);
    }

    L2Dir::L2Dir(L1Dir* parent, unsigned int l1, unsigned int l2)
      : freeindex(0)
      , freemask()
      , l1_dir(l1)
      , l2_dir(l2)
      , parent(parent)
    {
        this->freelist = (SquidFileHash*)SquidSnapshot::squidutils->_heap.alloc(
          sizeof(SquidFileHash) * L2_SIZE);

        this->freemask.set(0, L2_SIZE);

        Genode::Directory::Path path = to_path();
        SquidSnapshot::squidutils->createdir(path);

        for (unsigned int i = 0; i < L2_SIZE; i++) {
            construct_at<SquidFileHash>(&freelist[i], this, l1_dir, l2_dir, i);
        }
    }

    L2Dir::~L2Dir(void)
    {
        SquidSnapshot::squidutils->_heap.free(freelist, 0);
        parent->return_entry(l2_dir);
    }

    Genode::Directory::Path L2Dir::to_path(void)
    {
        Genode::size_t root_digits = (log2(ROOT_SIZE) / log2(16)) + 1;
        Genode::size_t l1_digits = (log2(L1_SIZE) / log2(16)) + 1;
        Genode::size_t total_chars =
          strlen("//current//") + strlen(SQUIDROOT) + root_digits + l1_digits;

        char path[total_chars];

        Format::snprintf(
          path, total_chars, "/%s/current/%x/%x", SQUIDROOT, l1_dir, l2_dir);

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

        Genode::error(SQUID_ERROR_FMT "This state should not be reacheable!");
        return nullptr;
    }

    void L2Dir::return_entry(unsigned int index)
    {
        freemask.set(index, 1);
    }

    SquidFileHash::SquidFileHash(L2Dir* parent,
                                 unsigned int l1,
                                 unsigned int l2,
                                 unsigned int file)
      : l1_dir(l1)
      , l2_dir(l2)
      , file_id(file)
      , parent(parent)
    {
    }

    SquidFileHash::~SquidFileHash(void)
    {
        parent->return_entry(file_id);
    }

    Genode::Directory::Path SquidFileHash::to_path(void)
    {
        Genode::size_t root_digits = (log2(ROOT_SIZE) / log2(16)) + 1;
        Genode::size_t l1_digits = (log2(L1_SIZE) / log2(16)) + 1;
        Genode::size_t l2_digits = (log2(L2_SIZE) / log2(16)) + 1;

        Genode::size_t total_chars = strlen("//current///") +
                                     strlen(SQUIDROOT) + root_digits +
                                     l1_digits + l2_digits;

        char hash[total_chars];

        Format::snprintf(hash,
                         total_chars,
                         "/%s/current/%x/%x/%x",
                         SquidSnapshot::SQUIDROOT,
                         l1_dir,
                         l2_dir,
                         file_id);

        return hash;
    }

    void SquidUtils::createdir(const Genode::Directory::Path& path)
    {
        Vfs::Vfs_handle* handle;
        auto res = _fs.opendir(path.string(), true, &handle, _heap);

        if (res != Vfs::Directory_service::OPENDIR_ERR_NODE_ALREADY_EXISTS &&
            res != Vfs::Directory_service::OPENDIR_OK) {
            Genode::error("Couldn't open directory: ", path);

            if (res == Vfs::Directory_service::OPENDIR_ERR_PERMISSION_DENIED)
                Genode::error("reason: permission");

            throw Genode::Exception();

            // } else {
            //     handle->close();
        }
    }

    Main::Main(SquidSnapshot::SquidUtils* squidutils)
    {
        // TODO finish this
        squidutils->createdir(Path("/squid-root/current"));

        construct_at<SquidSnapshot::SnapshotRoot>(&root_manager);
    }

    void Main::finish(void)
    {
        Genode::int64_t timestamp =
          SquidSnapshot::squidutils->_timer.curr_time()
            .trunc_to_plain_us()
            .value;

        char snapshot_timestamp[1024];
        Format::snprintf(
          snapshot_timestamp, 1024, "/%s/%llu", SQUIDROOT, timestamp);

        char snapshot_current[1024];
        Format::snprintf(snapshot_current, 1024, "/%s/current", SQUIDROOT);

        if (SquidSnapshot::squidutils->_fs.rename(snapshot_current,
                                                  snapshot_timestamp) ==
            Vfs::Directory_service::RENAME_ERR_NO_ENTRY) {
            Genode::error("rename no good!");
        }
    }

    Error Main::write(Path const& path, void* payload, size_t size)
    {
        try {
            New_file file(SquidSnapshot::squidutils->_root_dir, path);

            if (file.append((const char*)payload, size) !=
                New_file::Append_result::OK) {

                return Error::WriteFile;
            }

        } catch (New_file::Create_failed) {
            return Error::CreateFile;
        }

        return Error::None;
    }

    Error Main::read(Path const& path, void* payload)
    {
        Readonly_file file(SquidSnapshot::squidutils->_root_dir, path);
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

    Error Main::test(void)
    {
        char message[] = "payload";

        SquidFileHash* hash = global_squid->root_manager->get_hash();
        if (hash == nullptr)
            return Error::OutOfHashes;

        switch (global_squid->write(
          hash->to_path(), (void*)message, sizeof(message) / sizeof(char))) {

            case Error::CreateFile:
                return Error::CreateFile;

            case Error::WriteFile:
                return Error::WriteFile;

            default:
                break;
        }

        char* echo =
          (char*)SquidSnapshot::squidutils->_heap.alloc(sizeof(char) * 20);

        switch (
          SquidSnapshot::global_squid->read(hash->to_path(), (void*)echo)) {

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

        return Error::None;
    }
}; // namespace SquidSnapshot

#ifdef __cplusplus

extern "C"
{

    enum SquidError squid_hash(void** hash)
    {
        SquidSnapshot::SquidFileHash* squid_generated_hash =
          SquidSnapshot::global_squid->root_manager->get_hash();
        if (squid_generated_hash == nullptr)
            return SQUID_FULL;

        *hash = (void*)squid_generated_hash;
        return SQUID_NONE;
    }

    enum SquidError squid_write(void* hash,
                                void* payload,
                                unsigned long long size)
    {
        auto path = ((SquidSnapshot::SquidFileHash*)hash)->to_path();

        switch (SquidSnapshot::global_squid->write(path, payload, size)) {
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
        auto path = ((SquidSnapshot::SquidFileHash*)hash)->to_path();

        switch (SquidSnapshot::global_squid->read(path, payload)) {
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
            delete file;
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
}

#endif // __cplusplus
