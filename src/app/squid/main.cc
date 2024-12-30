#include <base/component.h>
#include <squid.h>
#include <benchmark.h>

extern SquidSnapshot::SquidUtils *squidutils;
extern SquidSnapshot::Main *global_squid;


void Component::construct(Genode::Env &env)
{

    static SquidSnapshot::SquidUtils local_squidutils { env };
    squidutils = &local_squidutils;

    static SquidSnapshot::Main local_squid;
    global_squid = &local_squid;
    
    
    Genode::log("testing squid...");

    SquidSnapshot::Error err = SquidSnapshot::global_squid->_test();
    switch (err) {
    case SquidSnapshot::Error::CreateFile:
	Genode::error("\nfailed to create file\n");
	break;

    case SquidSnapshot::Error::WriteFile:
	Genode::error("\nfailed to write\n");
	break;

    case SquidSnapshot::Error::ReadFile:
	Genode::error("\nfailed to read\n");
	break;

    case SquidSnapshot::Error::CorruptedFile:
	Genode::error("\nfile was corrupted\n");
	break;

    default:
	Genode::log("passed.\n");
	break;
    }

    Genode::log("benchmarking squid...");

    squid_benchmark();

    Genode::log("benchmark finished.");

}
