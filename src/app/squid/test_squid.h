#ifndef __TEST_SQUID_H
#define __TEST_SQUID_H

#include "squid.h"


void test_init_and_read() {

    vm_page me;
    me.id = 23;

    switch (squid_file_init("/test_init_and_read", &me, sizeof(me))) {
        case OpenFile:
            fprintf(stderr, "Couldn't create file!");
            exit(1);

        case WriteFile:
            fprintf(stderr, "Couldn't write to file!");
            exit(1);

        default:
            break;
    }

    vm_page tester;
    
    switch (squid_file_read("/test_init_and_read", &tester)) {
        case ReadFile:
            fprintf(stderr, "Couldn't read from file!");
            exit(1);

        default:
            break;
    }

    switch (squid_file_delete("/test_init_and_read")) {
    case DeleteFile:
	fprintf(stderr, "Couldn't delete file!");
	exit(1);

    default:
	break;
    }

    if (tester.id == me.id) return;
    else {
        fprintf(stderr, "vm_page id should be: %d, but is: %d!", me.id, tester.id);
	exit(1);
    }

}


#endif // __TEST_SQUID_H
