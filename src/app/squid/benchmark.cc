#include <benchmark.h>
#include <squid.h>

void squid_benchmark(void)
{
    struct big_data obj;
    obj.size = 100000;
    obj.width = 34234;
    obj.height = 253534;

    
    for (int i = 0; i < 10000; i++) {
	Genode::log("benchmark ", i);
	
	Squid_snapshot::SquidFileHash hash(Squid_snapshot::global_squid->availability_matrix);
	Squid_snapshot::global_squid->_write(hash.to_path(), (void*) &obj, sizeof(obj));
    }
}
