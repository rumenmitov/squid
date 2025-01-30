#include "util/construct_at.h"
#include <base/component.h>
#include <benchmark.h>
#include <squid.h>

SquidSnapshot::SquidUtils* SquidSnapshot::squidutils = nullptr;
SquidSnapshot::Main* SquidSnapshot::global_squid = nullptr;

void
Component::construct(Genode::Env& env)
{
    static SquidSnapshot::SquidUtils local_squidutils{ env };
    SquidSnapshot::squidutils = &local_squidutils;

    static SquidSnapshot::Main local_squid{ SquidSnapshot::squidutils };
    SquidSnapshot::global_squid = &local_squid;

    Genode::log("testing squid...");

    SquidSnapshot::Error err = SquidSnapshot::global_squid->test();
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
            Genode::log("passed.");
            break;
    }

    Genode::log("benchmarking squid...");

    squid_benchmark();
    SquidSnapshot::global_squid->finish();
    
    Genode::log("benchmark finished.");
}
