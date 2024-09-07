#ifndef __TEST_SQUID_H
#define __TEST_SQUID_H

#include "squid.h"

void test_hash() 
{
    if (squid_generate_hash() == NULL) {
	fprintf(stderr, "Couldn't generate hash!");
	exit(1);
    }
}


void test_init_and_read()
{
    vm_page me;
    strncpy(me.id, squid_generate_hash(), HASH_LEN);
    me.testing_secret = 23;

    char *path = (char*) malloc( sizeof(char) * 4096);
    strncpy(path, "/squid-cache/", strlen("/squid-cache/"));
    strncat(path, me.id, strlen(me.id));
    
    
    switch (squid_file_init(path, &me, sizeof(me))) {
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
    
    switch (squid_file_read(path, &tester)) {
    case ReadFile:
	fprintf(stderr, "Couldn't read from file!");
	exit(1);

    default:
	break;
    }

    switch (squid_file_delete(path)) {
    case DeleteFile:
	fprintf(stderr, "Couldn't delete file!");
	exit(1);

    default:
	break;
    }

    if ( strcmp(tester.testing_secret, me.testing_secret) == 0) return;
    else {
	fprintf(stderr, "vm_page secret should be: %s, but is: %s!",
		me.testing_secret, tester.testing_secret);
	exit(1);
    }

}


#endif // __TEST_SQUID_H
