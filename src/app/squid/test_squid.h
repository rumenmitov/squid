#pragma once

#include <base/log.h>
#include "squid.h"


bool test_write_read(Squid_snapshot::Main& main) {

    vm_page me;
    me.id = 23;

    vm_page tester;


    switch (main.init("/test_write_read", &me, sizeof(me))) {
        case Squid_snapshot::Error::CreateFile:
            Genode::error("Couldn't create file!");
            return false;

        case Squid_snapshot::Error::WriteFile:
            Genode::error("Couldn't write to file!");
            return false;

        default:
            break;
    }

    switch (main.get("/test_write_read", &tester)) {
        case Squid_snapshot::Error::ReadFile:
            Genode::error("Couldn't read from file!");
            return false;

        default:
            break;
    }

    if (tester.id == me.id) return true;
    else {
        Genode::error("id should be: ", me.id, " is: ", tester.id);
        return false;
    }

}
