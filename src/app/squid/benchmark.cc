#include <benchmark.h>
#include <squid.h>

void squid_benchmark(void)
{
    struct big_data obj;
    obj.size = 100000;
    obj.width = 34234;
    obj.height = 253534;

    
    for (int i = 0; i < 10000; i++) {
	char hash[Squid_snapshot::HASH_LEN];
	
	Squid_snapshot::global_squid->_hash(hash);
	Squid_snapshot::global_squid->_write(hash, (void*) &obj, sizeof(obj));
    }
}
