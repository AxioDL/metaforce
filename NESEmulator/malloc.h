#ifndef URDE_NESEMULATOR_MALLOC_H
#define URDE_NESEMULATOR_MALLOC_H

#ifdef __APPLE__
#include <stdlib.h>
#elif _WIN32
#include <../ucrt/malloc.h>
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winclude-next-absolute-path"
#include_next <malloc.h>
#pragma GCC diagnostic pop
#endif

#endif // URDE_NESEMULATOR_MALLOC_H
