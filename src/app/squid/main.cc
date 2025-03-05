#include "squidlib.h"
#include "util/construct_at.h"
#include <base/component.h>
#include <benchmark.h>
#include <squid.h>

#include <base/attached_rom_dataspace.h>
#include <base/buffered_output.h>
#include <base/component.h>
#include <base/heap.h>
#include <base/sleep.h>
#include <os/vfs.h>
#include <timer_session/connection.h>
#include <util/bit_array.h>
#include <vfs/file_system_factory.h>

SquidSnapshot::SquidUtils* SquidSnapshot::squidutils = nullptr;
SquidSnapshot::Main* SquidSnapshot::global_squid = nullptr;

void
Component::construct(Genode::Env& env)
{
    static SquidSnapshot::SquidUtils local_utils(env);
    SquidSnapshot::squidutils = &local_utils;

    static SquidSnapshot::Main local_squid(SquidSnapshot::squidutils);
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

    squid_init_snapshot();

    Genode::log("benchmarking squid...");

    squid_benchmark();

    Genode::log("benchmark finished.");

    squid_finish_snapshot();
}
