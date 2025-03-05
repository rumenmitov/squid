#include <benchmark.h>
#include <squidlib.h>

void
squid_benchmark(void)
{
    struct big_data obj;
    obj.size = 100000;
    obj.width = 34234;
    obj.height = 253534;

    squid_init_snapshot();
    
    for (Genode::uint64_t i = 0; i < 10000; i++) {
        Genode::log("benchmark ", i);

        char* filehash = nullptr;
        if (squid_hash((void**)&filehash) == SQUID_FULL) {
            Genode::error("SQUID: out of hashes: ", i);
            break;
        }

        if (squid_write(filehash, (void*)&obj, sizeof(obj)) != SQUID_NONE)
            Genode::error("SQUID: write: ", i);
    }

    squid_finish_snapshot();
}
