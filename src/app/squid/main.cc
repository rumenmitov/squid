#include <base/component.h>
#include <base/log.h>
#include <base/ram_allocator.h>

#include "squid.h"

void Component::construct(Genode::Env &env) {
    PH_snapshot::Main main(env);

    Genode::log("testing simple write to new file...");
//    vm_page hello;
//    vm_page *hello_ptr = &hello;
//    hello_ptr->id = 23;
//    char hello_ptr[] = "hello world!";
//    main._new_file("/hello", (void*) hello_ptr, sizeof(hello_ptr));
    main._read_file("/hello");
    Genode::log("passed.");
}
