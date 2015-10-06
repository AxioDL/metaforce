#ifndef _HECL_WINSUPPORT_H_
#define _HECL_WINSUPPORT_H_

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include "windows.h"

void* memmem(const void *haystack, size_t hlen, const void *needle, size_t nlen);
HRESULT CreateShellLink(LPCWSTR lpszPathObj, LPCWSTR lpszPathLink, LPCWSTR lpszDesc);
HRESULT ResolveShellLink(LPCWSTR lpszLinkFile, LPWSTR lpszPath, int iPathBufferSize);
bool TestShellLink(LPCWSTR lpszLinkFile);

#endif // _HECL_WINSUPPORT_H_
