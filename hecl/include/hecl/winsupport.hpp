#ifndef _HECL_WINSUPPORT_H_
#define _HECL_WINSUPPORT_H_

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#ifndef NOMINMAX
#define NOMINMAX 1
#endif
#include "windows.h"

void* memmem(const void *haystack, size_t hlen, const void *needle, size_t nlen);
int asprintf(char** buf, const char* format, ...);

#endif // _HECL_WINSUPPORT_H_
