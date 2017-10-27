#include "IOStreams.hpp"
#include "hecl/hecl.hpp"

namespace urde
{

#define DUMP_BITS 0

#if DUMP_BITS
static void PrintBinary(u32 val, u32 count)
{
    for (int i=0 ; i<count ; ++i)
    {
        printf("%d", (val >> (count-i-1)) & 0x1);
    }
}
#endif

/*!
 * \brief CBitStreamReader::ReadBit
 * Reads and decodes an encoded value from a bitstream.
 * \param bitCount How many bits to read
 * \return s32 The encoded value
 */
s32 CBitStreamReader::ReadEncoded(u32 bitCount)
{
#if DUMP_BITS
    auto pos = position();
    auto boff = x20_bitOffset;
#endif

    u32 ret = 0;
    s32 shiftAmt = x20_bitOffset - s32(bitCount);
    if (shiftAmt < 0)
    {
        /* OR in remaining bits of cached value */
        u32 mask = bitCount == 32 ? 0xffffffff : ((1 << bitCount) - 1);
        ret |= (x1c_val << u32(-shiftAmt)) & mask;

        /* Load in exact number of bytes remaining */
        auto loadDiv = std::div(-shiftAmt, 8);
        if (loadDiv.rem) ++loadDiv.quot;
        readUBytesToBuf(reinterpret_cast<u8*>(&x1c_val) + 4 - loadDiv.quot, loadDiv.quot);
        x1c_val = hecl::SBig(x1c_val);

        /* New bit offset */
        x20_bitOffset = loadDiv.quot * 8 + shiftAmt;

        /* OR in next bits */
        mask = (1 << u32(-shiftAmt)) - 1;
        ret |= (x1c_val >> x20_bitOffset) & mask;
    }
    else
    {
        /* OR in bits of cached value */
        u32 mask = bitCount == 32 ? 0xffffffff : ((1 << bitCount) - 1);
        ret |= (x1c_val >> u32(shiftAmt)) & mask;

        /* New bit offset */
        x20_bitOffset -= bitCount;
    }

#if DUMP_BITS
    printf("READ ");
    PrintBinary(ret, bitCount);
    printf(" %d %d\n", int(pos), int(boff));
#endif

    return ret;
}


void CBitStreamWriter::WriteEncoded(u32 val, u32 bitCount)
{
#if DUMP_BITS
    printf("WRITE ");
    PrintBinary(val, bitCount);
    printf(" %d %d\n", int(position()), int(x18_bitOffset));
#endif

    s32 shiftAmt = x18_bitOffset - s32(bitCount);
    if (shiftAmt < 0)
    {
        /* OR remaining bits to cached value */
        u32 mask = (1 << x18_bitOffset) - 1;
        x14_val |= (val >> u32(-shiftAmt)) & mask;

        /* Write out 32-bits */
        x14_val = hecl::SBig(x14_val);
        writeUBytes(reinterpret_cast<u8*>(&x14_val), 4);

        /* Cache remaining bits */
        x18_bitOffset = 0x20 + shiftAmt;
        x14_val = val << x18_bitOffset;
    }
    else
    {
        /* OR bits to cached value */
        u32 mask = bitCount == 32 ? 0xffffffff : ((1 << bitCount) - 1);
        x14_val |= (val & mask) << u32(shiftAmt);

        /* New bit offset */
        x18_bitOffset -= bitCount;
    }
}

void CBitStreamWriter::Flush()
{
    if (x18_bitOffset < 0x20)
    {
        auto pos = std::div(0x20 - x18_bitOffset, 8);
        if (pos.rem) ++pos.quot;
        x14_val = hecl::SBig(x14_val);
        writeUBytes(reinterpret_cast<u8*>(&x14_val), pos.quot);
        x18_bitOffset = 0x20;
        x14_val = 0;
    }
}

class CZipSupport
{
public:
    static void* Alloc(void*, u32 c, u32 n)
    {
        return new u8[c*n];
    }
    static void Free(void*, void* buf)
    {
        delete[] static_cast<u8*>(buf);
    }
};

CZipInputStream::CZipInputStream(std::unique_ptr<CInputStream>&& strm)
: x24_compBuf(new u8[4096]), x28_strm(std::move(strm))
{
    x30_zstrm.next_in = x24_compBuf.get();
    x30_zstrm.avail_in = 0;
    x30_zstrm.zalloc = CZipSupport::Alloc;
    x30_zstrm.zfree = CZipSupport::Free;
    inflateInit(&x30_zstrm);
}

CZipInputStream::~CZipInputStream()
{
    inflateEnd(&x30_zstrm);
}

atUint64 CZipInputStream::readUBytesToBuf(void *buf, atUint64 len)
{
    x30_zstrm.next_out = (Bytef*)buf;
    x30_zstrm.avail_out = len;
    while (x30_zstrm.avail_out != 0)
    {
        atUint64 readSz = x28_strm->readUBytesToBuf(x24_compBuf.get(), 4096);
        x30_zstrm.avail_in = readSz;
        x30_zstrm.next_in = x24_compBuf.get();
        if (inflate(&x30_zstrm, Z_NO_FLUSH) != Z_OK)
            break;
    }
    return x30_zstrm.total_out;
}

}
