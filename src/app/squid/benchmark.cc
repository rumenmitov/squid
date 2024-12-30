#include <benchmark.h>
#include <squidlib.h>

void squid_benchmark(void)
{
    struct big_data obj;
    obj.size = 100000;
    obj.width = 34234;
    obj.height = 253534;

    
    for (int i = 0; i < 10000; i++) {
	Genode::log("benchmark ", i);

	char *filehash = nullptr;
	squid_hash((void**) &filehash);
	
	if (squid_write(filehash, (void*) &obj, sizeof(obj)) != SQUID_NONE)
	    Genode::error("ERROR: write: ", i);
    }
}
