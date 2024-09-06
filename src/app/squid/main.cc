#include <base/component.h>
#include <base/log.h>
#include <base/ram_allocator.h>

#include "squid.h"
#include "test_squid.h"

void Component::construct(Genode::Env &env) {
    Squid_snapshot::Main main(env);

    Genode::log("Squid Cache Unit Tests:");
    if (test_write_read(main)) Genode::log("test_write_read...passed");
    else Genode::log("test_write_read...failed");
}
