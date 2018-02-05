#ifndef URDE_NESEMULATOR_MALLOC_H
#define URDE_NESEMULATOR_MALLOC_H

#ifdef __APPLE__
#include <stdlib.h>
#elif _WIN32
#include <../ucrt/malloc.h>
#else
#include_next <malloc.h>
#endif

#endif // URDE_NESEMULATOR_MALLOC_H
