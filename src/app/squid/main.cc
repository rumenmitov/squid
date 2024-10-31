#include <base/component.h>
#include <squid.h>

Squid_snapshot::Main *Squid_snapshot::global_squid = nullptr;

void Component::construct(Genode::Env &env)
{
    static Squid_snapshot::Main local_squid = Squid_snapshot::Main(env);
    Squid_snapshot::global_squid = &local_squid;

    Genode::log("testing squid...");

    Squid_snapshot::Error err = Squid_snapshot::global_squid->_test();
    switch (err) {
    case Squid_snapshot::Error::CreateFile:
	Genode::error("\nfailed to create file\n");
	break;

    case Squid_snapshot::Error::WriteFile:
	Genode::error("\nfailed to write\n");
	break;

    case Squid_snapshot::Error::ReadFile:
	Genode::error("\nfailed to read\n");
	break;

    case Squid_snapshot::Error::CorruptedFile:
	Genode::error("\nfile was corrupted\n");
	break;

    default:
	Genode::log("passed.\n");
	break;

    }

}
