#ifndef _HECL_WINSUPPORT_H_
#define _HECL_WINSUPPORT_H_

#if __cplusplus
extern "C" {
#endif

void* memmem(const void *haystack, size_t hlen, const void *needle, size_t nlen);

#if __cplusplus
}
#endif

#endif // _HECL_WINSUPPORT_H_
