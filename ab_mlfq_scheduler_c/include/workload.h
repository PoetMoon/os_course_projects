#ifndef WORKLOAD_H
#define WORKLOAD_H

#include "process.h"

int generate_workload(
    const char *scenario,
    int n,
    int seed,
    Process processes[]
);

#endif
