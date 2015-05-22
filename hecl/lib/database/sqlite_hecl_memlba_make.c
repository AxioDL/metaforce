#include "sqlite_hecl_vfs.h"
#include <zlib/zlib.h>
#include <stdint.h>
#include <stdbool.h>

/*
 * Block-compression LBA generator used for storing packed sqlite3 database
 */

#define BLOCK_SIZE 0x4000
#define ROUND_UP_BLOCK(val) (((val) + (BLOCK_SIZE-1)) & ~(BLOCK_SIZE-1))

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
static inline uint32_t makeu32(uint32_t val, bool bigEndian)
{
    if (!bigEndian)
        return val;
#if __GNUC__
    return __builtin_bswap32(val);
#elif _WIN32
    return _byteswap_ulong(val);
#else
    val = (val & 0x0000FFFF) << 16 | (val & 0xFFFF0000) >> 16;
    val = (val & 0x00FF00FF) << 8 | (val & 0xFF00FF00) >> 8;
    return val;
#endif
}
#else
static inline uint32_t makeu32(uint32_t val, bool bigEndian)
{
    if (bigEndian)
        return val;
#if __GNUC__
    return __builtin_bswap32(val);
#elif _WIN32
    return _byteswap_ulong(val);
#else
    val = (val & 0x0000FFFF) << 16 | (val & 0xFFFF0000) >> 16;
    val = (val & 0x00FF00FF) << 8 | (val & 0xFF00FF00) >> 8;
    return val;
#endif
}
#endif

/* Useful macros used in several places */
#define MIN(x,y) ((x)<(y)?(x):(y))
#define MAX(x,y) ((x)>(y)?(x):(y))

void sqlite_hecl_memlba_make(FILE* fout, void* bufin, size_t bufinLen, bool bigEndian)
{
    unsigned i;

    /* Magic */
    fwrite("HLPK", 1, 4, fout);

    /* Block size */
    uint32_t blockSize_s = makeu32(BLOCK_SIZE, bigEndian);
    fwrite(&blockSize_s, 1, 4, fout);

    /* Block count */
    size_t blockCount = ROUND_UP_BLOCK(bufinLen) / BLOCK_SIZE;
    uint32_t blockCount_s = makeu32(blockCount, bigEndian);
    fwrite(&blockCount_s, 1, 4, fout);

    /* Header+TOC+DB Size */
    fwrite("\0\0\0\0", 1, 4, fout);

    /* Block TOC buffer and file-space */
    uint32_t* blockTOC = calloc(blockCount+1, 4);
    for (i=0 ; i<blockCount+1 ; ++i)
        fwrite("\0\0\0\0", 1, 4, fout);

    /* Block-compression context */
    z_stream zstrm = {};
    deflateInit(&zstrm, 7);
    zstrm.next_in = bufin;

    /* Compress! */
    size_t curOff = 16 + (blockCount+1) * 4;
    size_t remSz = bufinLen;
    for (i=0 ; i<blockCount ; ++i)
    {
        unsigned char compBuf[BLOCK_SIZE*2];
        zstrm.avail_in = MIN(remSz, BLOCK_SIZE);
        zstrm.avail_out = BLOCK_SIZE*2;
        zstrm.next_out = compBuf;
        deflate(&zstrm, Z_FINISH);
        fwrite(compBuf, 1, zstrm.total_out, fout);
        blockTOC[i] = makeu32(curOff, bigEndian);
        curOff += zstrm.total_out;
        remSz -= zstrm.total_in;
        deflateReset(&zstrm);
    }

    /* Write Header+TOC+DB Size */
    fseek(fout, 12, SEEK_SET);
    uint32_t headSz_s = makeu32(curOff, bigEndian);
    fwrite(&headSz_s, 1, 4, fout);

    /* Write TOC */
    blockTOC[blockCount] = headSz_s;
    fwrite(blockTOC, 4, blockCount+1, fout);

    /* Cleanup */
    deflateEnd(&zstrm);
    free(blockTOC);

}
