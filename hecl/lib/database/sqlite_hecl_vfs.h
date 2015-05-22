#ifndef SQLITE_HECL_VFS
#define SQLITE_HECL_VFS

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef void(*TCloseCallback)(void* buf, size_t bufLen, void* ctx);
int sqlite_hecl_mem_vfs_register(TCloseCallback closeCb, void* ctx);
int sqlite_hecl_memlba_vfs_register(void* headBuf);
void sqlite_hecl_memlba_make(FILE* fout, void* bufin, size_t bufinLen, bool bigEndian);

#ifdef __cplusplus
}
#endif

#endif // SQLITE_HECL_VFS
