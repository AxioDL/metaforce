#include "sqlite_hecl_vfs.h"

#include "sqlite3.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * Modified test_onefile.c VFS for sqlite3
 *
 * This VFS gets registered with sqlite to access an in-memory,
 * block-compressed LBA. It's designed for read-only access of
 * a packed object-database.
 *
 * Journal and temp read/write is unsupported and will call abort
 * if attempted.
 */

/*
 * App-supplied callback called when file is closed (ready to access)
 */
static TCloseCallback CLOSE_CALLBACK = NULL;
static void* CLOSE_CTX = NULL;

typedef struct mem_file mem_file;
struct mem_file {
    sqlite3_file base;
    int nSize;
    int nAlloc;
    char *zAlloc;
};

/*
** Method declarations for mem_file.
*/
static int memClose(sqlite3_file*);
static int memRead(sqlite3_file*, void*, int iAmt, sqlite3_int64 iOfst);
static int memWrite(sqlite3_file*, const void*, int iAmt, sqlite3_int64 iOfst);
static int memTruncate(sqlite3_file*, sqlite3_int64 size);
static int memSync(sqlite3_file*, int flags);
static int memFileSize(sqlite3_file*, sqlite3_int64 *pSize);
static int memLock(sqlite3_file*, int);
static int memUnlock(sqlite3_file*, int);
static int memCheckReservedLock(sqlite3_file*, int *pResOut);
static int memFileControl(sqlite3_file*, int op, void *pArg);
static int memSectorSize(sqlite3_file*);
static int memDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for fs_vfs.
*/
static int memOpen(sqlite3_vfs*, const char *, sqlite3_file*, int , int *);
static int memDelete(sqlite3_vfs*, const char *zName, int syncDir);
static int memAccess(sqlite3_vfs*, const char *zName, int flags, int *);
static int memFullPathname(sqlite3_vfs*, const char *zName, int nOut,char *zOut);
static void *memDlOpen(sqlite3_vfs*, const char *zFilename);
static void memDlError(sqlite3_vfs*, int nByte, char *zErrMsg);
static void (*memDlSym(sqlite3_vfs*,void*, const char *zSymbol))(void);
static void memDlClose(sqlite3_vfs*, void*);
static int memRandomness(sqlite3_vfs*, int nByte, char *zOut);
static int memSleep(sqlite3_vfs*, int microseconds);
static int memCurrentTime(sqlite3_vfs*, double*);

static sqlite3_vfs mem_vfs = {
    1,                                          /* iVersion */
    0,                                          /* szOsFile */
    0,                                          /* mxPathname */
    0,                                          /* pNext */
    "hecl_mem",                                 /* zName */
    0,                                          /* pAppData */
    memOpen,                                     /* xOpen */
    memDelete,                                   /* xDelete */
    memAccess,                                   /* xAccess */
    memFullPathname,                             /* xFullPathname */
    memDlOpen,                                   /* xDlOpen */
    memDlError,                                  /* xDlError */
    memDlSym,                                    /* xDlSym */
    memDlClose,                                  /* xDlClose */
    memRandomness,                               /* xRandomness */
    memSleep,                                    /* xSleep */
    memCurrentTime,                              /* xCurrentTime */
    0                                           /* xCurrentTimeInt64 */
};

static sqlite3_io_methods mem_io_methods = {
    1,                            /* iVersion */
    memClose,                     /* xClose */
    memRead,                      /* xRead */
    memWrite,                     /* xWrite */
    memTruncate,                  /* xTruncate */
    memSync,                      /* xSync */
    memFileSize,                  /* xFileSize */
    memLock,                      /* xLock */
    memUnlock,                    /* xUnlock */
    memCheckReservedLock,         /* xCheckReservedLock */
    memFileControl,               /* xFileControl */
    memSectorSize,                /* xSectorSize */
    memDeviceCharacteristics,     /* xDeviceCharacteristics */
    0,                            /* xShmMap */
    0,                            /* xShmLock */
    0,                            /* xShmBarrier */
    0,                            /* xShmUnmap */
    0,
    0
};

/* Useful macros used in several places */
#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))


/*
** Close a mem-file.
*/
static int memClose(sqlite3_file *pFile){
    mem_file *pTmp = (mem_file *)pFile;
    if (CLOSE_CALLBACK)
        CLOSE_CALLBACK(pTmp->zAlloc, pTmp->nSize, CLOSE_CTX);
    else
        sqlite3_free(pTmp->zAlloc);
    return SQLITE_OK;
}

/*
** Read data from a mem-file.
*/
static int memRead(
        sqlite3_file *pFile,
        void *zBuf,
        int iAmt,
        sqlite_int64 iOfst
        ){
    mem_file *pTmp = (mem_file *)pFile;
    if( (iAmt+iOfst)>pTmp->nSize ){
        return SQLITE_IOERR_SHORT_READ;
    }
    memcpy(zBuf, &pTmp->zAlloc[iOfst], iAmt);
    return SQLITE_OK;
}

/*
** Write data to a mem-file.
*/
static int memWrite(
        sqlite3_file *pFile,
        const void *zBuf,
        int iAmt,
        sqlite_int64 iOfst
        ){
    mem_file *pTmp = (mem_file *)pFile;
    if( (iAmt+iOfst)>pTmp->nAlloc ){
        int nNew = (int)(2*(iAmt+iOfst+pTmp->nAlloc));
        char *zNew = sqlite3_realloc(pTmp->zAlloc, nNew);
        if( !zNew ){
            return SQLITE_NOMEM;
        }
        pTmp->zAlloc = zNew;
        pTmp->nAlloc = nNew;
    }
    memcpy(&pTmp->zAlloc[iOfst], zBuf, iAmt);
    pTmp->nSize = (int)MAX(pTmp->nSize, iOfst+iAmt);
    return SQLITE_OK;
}

/*
** Truncate a mem-file.
*/
static int memTruncate(sqlite3_file *pFile, sqlite_int64 size){
    mem_file *pTmp = (mem_file *)pFile;
    pTmp->nSize = (int)MIN(pTmp->nSize, size);
    return SQLITE_OK;
}

/*
** Sync a mem-file.
*/
static int memSync(sqlite3_file *pFile, int flags){
    return SQLITE_OK;
}

/*
** Return the current file-size of a mem-file.
*/
static int memFileSize(sqlite3_file *pFile, sqlite_int64 *pSize){
    mem_file *pTmp = (mem_file *)pFile;
    *pSize = pTmp->nSize;
    return SQLITE_OK;
}

/*
** Lock a mem-file.
*/
static int memLock(sqlite3_file *pFile, int eLock){
    return SQLITE_OK;
}

/*
** Unlock a mem-file.
*/
static int memUnlock(sqlite3_file *pFile, int eLock){
    return SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on a mem-file.
*/
static int memCheckReservedLock(sqlite3_file *pFile, int *pResOut){
    *pResOut = 0;
    return SQLITE_OK;
}

/*
** File control method. For custom operations on a mem-file.
*/
static int memFileControl(sqlite3_file *pFile, int op, void *pArg){
    return SQLITE_OK;
}

/*
** Return the sector-size in bytes for a mem-file.
*/
static int memSectorSize(sqlite3_file *pFile){
    return 0;
}

/*
** Return the device characteristic flags supported by a mem-file.
*/
static int memDeviceCharacteristics(sqlite3_file *pFile){
    return 0;
}

/*
** Open an mem file handle.
*/
static int memOpen(
        sqlite3_vfs *pVfs,
        const char *zName,
        sqlite3_file *pFile,
        int flags,
        int *pOutFlags
        ){
    if (flags != SQLITE_OPEN_MAIN_DB)
    {
        fprintf(stderr, "the sqlite hecl mem VFS only supports main-db writing\n");
        abort();
    }
    mem_file *p2 = (mem_file *)pFile;
    memset(p2, 0, sizeof(*p2));
    p2->base.pMethods = &mem_io_methods;
    return SQLITE_OK;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
static int memDelete(sqlite3_vfs *pVfs, const char *zPath, int dirSync){
    return SQLITE_OK;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
static int memAccess(
        sqlite3_vfs *pVfs,
        const char *zPath,
        int flags,
        int *pResOut
        ){
    if (flags & SQLITE_ACCESS_READ | SQLITE_ACCESS_READWRITE)
        return 1;
    return 0;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (FS_MAX_PATHNAME+1) bytes.
*/
static int memFullPathname(
        sqlite3_vfs *pVfs,            /* Pointer to vfs object */
        const char *zPath,            /* Possibly relative input path */
        int nOut,                     /* Size of output buffer in bytes */
        char *zOut                    /* Output buffer */
        ){
    strncpy(zOut, zPath, nOut);
    return SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
static void *memDlOpen(sqlite3_vfs *pVfs, const char *zPath){
    return NULL;
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated
** with dynamic libraries.
*/
static void memDlError(sqlite3_vfs *pVfs, int nByte, char *zErrMsg){
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
static void (*memDlSym(sqlite3_vfs *pVfs, void *pH, const char *zSym))(void){
}

/*
** Close the dynamic library handle pHandle.
*/
static void memDlClose(sqlite3_vfs *pVfs, void *pHandle){
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of
** random data.
*/
static int memRandomness(sqlite3_vfs *pVfs, int nByte, char *zBufOut){
    for (int i=0 ; i<nByte ; ++i)
        zBufOut[i] = rand();
    return nByte;
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds
** actually slept.
*/
static int memSleep(sqlite3_vfs *pVfs, int nMicro){
    int seconds = (nMicro+999999)/1000000;
    sleep(seconds);
    return seconds*1000000;
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
static int memCurrentTime(sqlite3_vfs *pVfs, double *pTimeOut){
    *pTimeOut = 0.0;
    return 0;
}

/*
** This procedure registers the mem vfs with SQLite. If the argument is
** true, the mem vfs becomes the new default vfs. It is the only publicly
** available function in this file.
*/
int sqlite_hecl_mem_vfs_register(TCloseCallback closeCb, void* ctx){
    CLOSE_CALLBACK = closeCb;
    CLOSE_CTX = ctx;
    if (mem_vfs.szOsFile) return SQLITE_OK;
    mem_vfs.szOsFile = sizeof(mem_file);
    return sqlite3_vfs_register(&mem_vfs, 0);
}
