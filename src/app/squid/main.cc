#include <base/component.h>
#include <base/log.h>
#include <base/ram_allocator.h>

#include "squid.h"

void Component::construct(Genode::Env &env) {
    Squid_snapshot::Main main(env);

    Genode::log("testing simple write to new file...");
    vm_page hello;
    main.init_squid_file("/hello", &hello, sizeof(hello));

    vm_page hello_check;
    main.read_squid_file("/hello", &hello_check);
    Genode::log("passed.");

    Genode::log("\ntesting timestamp...");
    Genode::log("timestamp: ", Squid_snapshot::Main::get_hash());
    Genode::log("passed.");
}
