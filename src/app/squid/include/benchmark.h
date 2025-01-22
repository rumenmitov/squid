#ifndef __BENCHMARK_H
#define __BENCHMARK_H

#include "squid.h"

struct big_data
{
    unsigned int size;
    unsigned int width;
    unsigned int height;
};

void squid_benchmark (void);

#endif // __BENCHMARK_H
