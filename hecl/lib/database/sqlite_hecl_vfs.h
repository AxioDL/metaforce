#ifndef SQLITE_HECL_VFS
#define SQLITE_HECL_VFS

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
typedef void(*TCloseCallback)(void* buf, size_t bufLen, void* ctx);
int sqlite_hecl_mem_vfs_register(TCloseCallback closeCb, void* ctx);
int sqlite_hecl_memlba_vfs_register(TCloseCallback closeCb, void* ctx);

#ifdef __cplusplus
}
#endif

#endif // SQLITE_HECL_VFS
