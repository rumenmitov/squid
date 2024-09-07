#include "squid.h"
#include "test_squid.h"

int main(int, char **)
{
    test_hash();
    test_init_and_read();
    
    return 0;
}
