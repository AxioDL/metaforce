#include "sqlite_hecl_vfs.h"

#include "sqlite3.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include <zlib/zlib.h>

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
 * App-supplied pointer to head buffer (Header+TOC+DB)
 */
static void* HEAD_BUF = NULL;

#define BLOCK_SLOTS 4

typedef struct memlba_file memlba_file;
struct memlba_file
{
    sqlite3_file base;
    struct
    {
        char magic[4];
        uint32_t blockSize;
        uint32_t blockCount;
        uint32_t headSz;
        uint32_t blockTOC[];
    }* headBuf;
    void* cachedBlockBufs[BLOCK_SLOTS];
    /* All initialized to -1 */
    int cachedBlockIndices[BLOCK_SLOTS];
    /* Ages start at 0, newly inserted block is 1.
     * Non-0 blocks incremented on every insertion.
     * If any slot is BLOCK_SLOTS upon insertion, surrounding blocks
     * are incremented and that slot is reset to 1 (oldest-block caching)
     */
    int cachedBlockAges[BLOCK_SLOTS];
    z_stream zstrm;
};

static int newBlockSlot(memlba_file* file)
{
    unsigned i;
    for (i=0 ; i<BLOCK_SLOTS ; ++i)
        if (file->cachedBlockAges[i] != 0)
            ++file->cachedBlockAges[i];
    for (i=0 ; i<BLOCK_SLOTS ; ++i)
        if (file->cachedBlockAges[i] == 0)
        {
            file->cachedBlockAges[i] = 1;
            return i;
        }
    for (i=0 ; i<BLOCK_SLOTS ; ++i)
        if (file->cachedBlockAges[i] == BLOCK_SLOTS+1)
        {
            file->cachedBlockAges[i] = 1;
            return i;
        }
    /* Shouldn't happen (fallback) */
    for (i=1 ; i<BLOCK_SLOTS ; ++i)
    {
        file->cachedBlockAges[i] = 0;
        file->cachedBlockIndices[i] = -1;
    }
    file->cachedBlockAges[0] = 1;
    return 0;
}

static void decompressBlock(memlba_file* file, int blockIdx, int targetSlot)
{
    if (blockIdx >= file->headBuf->blockCount)
    {
        fprintf(stderr, "exceeded memlba block range");
        abort();
    }
    void* dbBlock = ((void*)file->headBuf) + file->headBuf->blockTOC[blockIdx];
    file->zstrm.next_in = dbBlock;
    file->zstrm.avail_in = file->headBuf->blockTOC[blockIdx+1] - file->headBuf->blockTOC[blockIdx];
    file->zstrm.next_out = file->cachedBlockBufs[targetSlot];
    file->zstrm.avail_out = file->headBuf->blockSize;
    inflate(&file->zstrm, Z_FINISH);
    inflateReset(&file->zstrm);
}

static int getBlockSlot(memlba_file* file, int blockIdx)
{
    unsigned i;
    for (i=0 ; i<BLOCK_SLOTS ; ++i)
        if (file->cachedBlockIndices[i] != blockIdx)
            return i;
    int newSlot = newBlockSlot(file);
    file->cachedBlockIndices[newSlot] = blockIdx;
    decompressBlock(file, blockIdx, newSlot);
    return newSlot;
}

/*
** Method declarations for memlba_file.
*/
static int memlbaClose(sqlite3_file*);
static int memlbaRead(sqlite3_file*, void*, int iAmt, sqlite3_int64 iOfst);
static int memlbaWrite(sqlite3_file*, const void*, int iAmt, sqlite3_int64 iOfst);
static int memlbaTruncate(sqlite3_file*, sqlite3_int64 size);
static int memlbaSync(sqlite3_file*, int flags);
static int memlbaFileSize(sqlite3_file*, sqlite3_int64* pSize);
static int memlbaLock(sqlite3_file*, int);
static int memlbaUnlock(sqlite3_file*, int);
static int memlbaCheckReservedLock(sqlite3_file*, int* pResOut);
static int memlbaFileControl(sqlite3_file*, int op, void* pArg);
static int memlbaSectorSize(sqlite3_file*);
static int memlbaDeviceCharacteristics(sqlite3_file*);

/*
** Method declarations for fs_vfs.
*/
static int memlbaOpen(sqlite3_vfs*, const char*, sqlite3_file*, int , int*);
static int memlbaDelete(sqlite3_vfs*, const char* zName, int syncDir);
static int memlbaAccess(sqlite3_vfs*, const char* zName, int flags, int*);
static int memlbaFullPathname(sqlite3_vfs*, const char* zName, int nOut, char* zOut);
static void* memlbaDlOpen(sqlite3_vfs*, const char* zFilename);
static void memlbaDlError(sqlite3_vfs*, int nByte, char* zErrMsg);
static void (*memlbaDlSym(sqlite3_vfs*, void*, const char* zSymbol))(void);
static void memlbaDlClose(sqlite3_vfs*, void*);
static int memlbaRandomness(sqlite3_vfs*, int nByte, char* zOut);
static int memlbaSleep(sqlite3_vfs*, int microseconds);
static int memlbaCurrentTime(sqlite3_vfs*, double*);

static sqlite3_vfs memlba_vfs =
{
    1,                                          /* iVersion */
    0,                                          /* szOsFile */
    0,                                          /* mxPathname */
    0,                                          /* pNext */
    "hecl_memlba",                                 /* zName */
    0,                                          /* pAppData */
    memlbaOpen,                                     /* xOpen */
    memlbaDelete,                                   /* xDelete */
    memlbaAccess,                                   /* xAccess */
    memlbaFullPathname,                             /* xFullPathname */
    memlbaDlOpen,                                   /* xDlOpen */
    memlbaDlError,                                  /* xDlError */
    memlbaDlSym,                                    /* xDlSym */
    memlbaDlClose,                                  /* xDlClose */
    memlbaRandomness,                               /* xRandomness */
    memlbaSleep,                                    /* xSleep */
    memlbaCurrentTime,                              /* xCurrentTime */
    0                                           /* xCurrentTimeInt64 */
};

static sqlite3_io_methods memlba_io_methods =
{
    1,                            /* iVersion */
    memlbaClose,                     /* xClose */
    memlbaRead,                      /* xRead */
    memlbaWrite,                     /* xWrite */
    memlbaTruncate,                  /* xTruncate */
    memlbaSync,                      /* xSync */
    memlbaFileSize,                  /* xFileSize */
    memlbaLock,                      /* xLock */
    memlbaUnlock,                    /* xUnlock */
    memlbaCheckReservedLock,         /* xCheckReservedLock */
    memlbaFileControl,               /* xFileControl */
    memlbaSectorSize,                /* xSectorSize */
    memlbaDeviceCharacteristics,     /* xDeviceCharacteristics */
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
** Close a memlba-file.
*/
static int memlbaClose(sqlite3_file* pFile)
{
    memlba_file* pTmp = (memlba_file*)pFile;
    free(pTmp->headBuf);
    free(pTmp->cachedBlockBufs[0]);
    inflateEnd(&pTmp->zstrm);
    return SQLITE_OK;
}

/*
** Read data from a memlba-file.
*/
static int memlbaRead(
    sqlite3_file* pFile,
    void* zBuf,
    int iAmt,
    sqlite_int64 iOfst
)
{
    memlba_file* pTmp = (memlba_file*)pFile;

    unsigned blockIdx = iOfst / pTmp->headBuf->blockSize;
    unsigned firstOff = iOfst % pTmp->headBuf->blockSize;
    unsigned firstRemBytes = pTmp->headBuf->blockSize - firstOff;

    int slot = getBlockSlot(pTmp, blockIdx);
    unsigned toRead = MIN(iAmt, firstRemBytes);
    memcpy(zBuf, pTmp->cachedBlockBufs[slot] + firstOff, toRead);
    iAmt -= toRead;
    zBuf += toRead;

    while (iAmt)
    {
        slot = getBlockSlot(pTmp, ++blockIdx);
        toRead = MIN(iAmt, pTmp->headBuf->blockSize);
        memcpy(zBuf, pTmp->cachedBlockBufs[slot], toRead);
        iAmt -= toRead;
        zBuf += toRead;
    }

    return SQLITE_OK;
}

/*
** Write data to a memlba-file.
*/
static int memlbaWrite(
    sqlite3_file* pFile,
    const void* zBuf,
    int iAmt,
    sqlite_int64 iOfst
)
{
    return SQLITE_OK;
}

/*
** Truncate a memlba-file.
*/
static int memlbaTruncate(sqlite3_file* pFile, sqlite_int64 size)
{
    memlba_file* pTmp = (memlba_file*)pFile;
    return SQLITE_OK;
}

/*
** Sync a memlba-file.
*/
static int memlbaSync(sqlite3_file* pFile, int flags)
{
    return SQLITE_OK;
}

/*
** Return the current file-size of a memlba-file.
*/
static int memlbaFileSize(sqlite3_file* pFile, sqlite_int64* pSize)
{
    memlba_file* pTmp = (memlba_file*)pFile;
    *pSize = pTmp->headBuf->headSz - pTmp->headBuf->blockTOC[0];
    return SQLITE_OK;
}

/*
** Lock a memlba-file.
*/
static int memlbaLock(sqlite3_file* pFile, int eLock)
{
    return SQLITE_OK;
}

/*
** Unlock a memlba-file.
*/
static int memlbaUnlock(sqlite3_file* pFile, int eLock)
{
    return SQLITE_OK;
}

/*
** Check if another file-handle holds a RESERVED lock on a memlba-file.
*/
static int memlbaCheckReservedLock(sqlite3_file* pFile, int* pResOut)
{
    *pResOut = 0;
    return SQLITE_OK;
}

/*
** File control method. For custom operations on a memlba-file.
*/
static int memlbaFileControl(sqlite3_file* pFile, int op, void* pArg)
{
    return SQLITE_OK;
}

/*
** Return the sector-size in bytes for a memlba-file.
*/
static int memlbaSectorSize(sqlite3_file* pFile)
{
    return 0;
}

/*
** Return the device characteristic flags supported by a memlba-file.
*/
static int memlbaDeviceCharacteristics(sqlite3_file* pFile)
{
    return 0;
}

/*
** Open an memlba file handle.
*/
static int memlbaOpen(
    sqlite3_vfs* pVfs,
    const char* zName,
    sqlite3_file* pFile,
    int flags,
    int* pOutFlags
)
{
    if ((flags & SQLITE_OPEN_MAIN_DB) != SQLITE_OPEN_MAIN_DB ||
        (flags & SQLITE_OPEN_READONLY) != SQLITE_OPEN_READONLY)
    {
        fprintf(stderr, "the sqlite hecl memlba VFS only supports main-db reading\n");
        return SQLITE_CANTOPEN;
    }
    memlba_file* p2 = (memlba_file*)pFile;
    memset(p2, 0, sizeof(*p2));
    p2->base.pMethods = &memlba_io_methods;
    inflateInit(&p2->zstrm);
    p2->headBuf = HEAD_BUF;
    unsigned i;
    void* blockBufs = calloc(BLOCK_SLOTS, p2->headBuf->blockSize);
    for (i=0 ; i<BLOCK_SLOTS ; ++i)
    {
        p2->cachedBlockBufs[i] = blockBufs + p2->headBuf->blockSize * i;
    }
    return SQLITE_OK;
}

/*
** Delete the file located at zPath. If the dirSync argument is true,
** ensure the file-system modifications are synced to disk before
** returning.
*/
static int memlbaDelete(sqlite3_vfs* pVfs, const char* zPath, int dirSync)
{
    return SQLITE_OK;
}

/*
** Test for access permissions. Return true if the requested permission
** is available, or false otherwise.
*/
static int memlbaAccess(
    sqlite3_vfs* pVfs,
    const char* zPath,
    int flags,
    int* pResOut
)
{
    if(flags & SQLITE_ACCESS_READ | SQLITE_ACCESS_READWRITE)
        return 1;
    return 0;
}

/*
** Populate buffer zOut with the full canonical pathname corresponding
** to the pathname in zPath. zOut is guaranteed to point to a buffer
** of at least (FS_MAX_PATHNAME+1) bytes.
*/
static int memlbaFullPathname(
    sqlite3_vfs* pVfs,            /* Pointer to vfs object */
    const char* zPath,            /* Possibly relative input path */
    int nOut,                     /* Size of output buffer in bytes */
    char* zOut)                   /* Output buffer */
{
    strncpy(zOut, zPath, nOut);
    return SQLITE_OK;
}

/*
** Open the dynamic library located at zPath and return a handle.
*/
static void* memlbaDlOpen(sqlite3_vfs* pVfs, const char* zPath)
{
    return NULL;
}

/*
** Populate the buffer zErrMsg (size nByte bytes) with a human readable
** utf-8 string describing the most recent error encountered associated
** with dynamic libraries.
*/
static void memlbaDlError(sqlite3_vfs* pVfs, int nByte, char* zErrMsg)
{
}

/*
** Return a pointer to the symbol zSymbol in the dynamic library pHandle.
*/
static void (*memlbaDlSym(sqlite3_vfs* pVfs, void* pH, const char* zSym))(void)
{
}

/*
** Close the dynamic library handle pHandle.
*/
static void memlbaDlClose(sqlite3_vfs* pVfs, void* pHandle)
{
}

/*
** Populate the buffer pointed to by zBufOut with nByte bytes of
** random data.
*/
static int memlbaRandomness(sqlite3_vfs* pVfs, int nByte, char* zBufOut)
{
    for(int i = 0 ; i < nByte ; ++i)
        zBufOut[i] = rand();
    return nByte;
}

/*
** Sleep for nMicro microseconds. Return the number of microseconds
** actually slept.
*/
static int memlbaSleep(sqlite3_vfs* pVfs, int nMicro)
{
    int seconds = (nMicro + 999999) / 1000000;
    sleep(seconds);
    return seconds * 1000000;
}

/*
** Return the current time as a Julian Day number in *pTimeOut.
*/
static int memlbaCurrentTime(sqlite3_vfs* pVfs, double* pTimeOut)
{
    *pTimeOut = 0.0;
    return 0;
}

/*
** This procedure registers the memlba vfs with SQLite. If the argument is
** true, the memlba vfs becomes the new default vfs. It is the only publicly
** available function in this file.
*/
int sqlite_hecl_memlba_vfs_register(void* headBuf)
{
    HEAD_BUF = headBuf;
    if(memlba_vfs.szOsFile) return SQLITE_OK;
    memlba_vfs.szOsFile = sizeof(memlba_file);
    return sqlite3_vfs_register(&memlba_vfs, 0);
}
