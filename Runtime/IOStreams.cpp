#include "IOStreams.hpp"

namespace urde
{
/*!
 * \brief CBitStreamReader::ReadBit
 * Reads and decodes an encoded value from a bitstream.
 * \param bitCount How many bits to read
 * \return s32 The encoded value
 */
s32 CBitStreamReader::ReadEncoded(u32 bitCount)
{
    s32 ret = 0;
    if (bitCount < x20_bitOffset)
    {
        u32 diff = 0x20 - bitCount;
        u32 baseVal = -1;
        if (x20_bitOffset != 0x20)
            baseVal = (1 << bitCount) - 1;
        x20_bitOffset -= bitCount;
        ret = baseVal & (x1c_val >> diff);
        x1c_val <<= bitCount;
    }
    else
    {
        u32 diff = bitCount - x20_bitOffset;
        u32 baseVal = -1;
        if (x20_bitOffset != 32)
            baseVal = (1 << x20_bitOffset) - 1;

        baseVal = (baseVal & (x1c_val >> (32 - x20_bitOffset))) << diff;
        x20_bitOffset = 0;
        auto pos = std::div(diff, 8);
        if (pos.rem) ++pos.quot;
        readUBytesToBuf(&x1c_val, pos.quot);
        /* The game uses Big Endian, which doesn't work for us */
        /* Little Endian sucks */
        athena::utility::BigUint32(x1c_val);

        u32 baseVal2 = -1;
        if (diff != 32)
            baseVal2 = (1 << diff) - 1;

        ret = baseVal | (baseVal2 & (x1c_val >> (32 - diff))) << x20_bitOffset;
        x20_bitOffset = (pos.quot << 3) - diff;
        x1c_val <<= diff;
    }

    return ret;
}


void CBitStreamWriter::WriteEncoded(u32 val, u32 bitCount)
{
    if (x18_bitOffset >= bitCount)
    {
        int baseVal = -1;
        if (bitCount != 32)
            baseVal = (1 << bitCount) - 1;
        x14_val |= (val & baseVal) << (x18_bitOffset - bitCount);
        x18_bitOffset -= bitCount;
    }
    else
    {
        u32 diff = bitCount - x18_bitOffset;
        u32 baseVal = -1;
        if (x18_bitOffset != 32)
            baseVal = (1 << x18_bitOffset) - 1;

        x14_val |= (val >> diff) & baseVal;
        x18_bitOffset = 0;
        u32 tmp = x14_val;
        athena::utility::BigUint32(tmp);
        auto pos = std::div(32 - x18_bitOffset, 8);
        if (pos.rem) ++pos.quot;
        writeBytes(&tmp, pos.quot);

        u32 rem = 32 - diff;
        baseVal = -1;
        if (diff != 32)
            baseVal = (1 << diff) - 1;

        x14_val = (val & baseVal) << rem;
        x18_bitOffset -= diff;
    }
}

void CBitStreamWriter::Flush()
{
    if (x18_bitOffset && x18_bitOffset < 0x20)
    {
        u32 tmp = x14_val;
        athena::utility::BigUint32(tmp);
        auto pos = std::div(32 - x18_bitOffset, 8);
        if (pos.rem) ++pos.quot;
        writeBytes(&tmp, pos.quot);
        x18_bitOffset = 32;
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
    if (!x30_zstrm.avail_in)
    {
        atUint64 readSz = x28_strm->readUBytesToBuf(x24_compBuf.get(), 4096);
        x30_zstrm.avail_in = readSz;
        x30_zstrm.next_in = x24_compBuf.get();
    }
    inflate(&x30_zstrm, Z_NO_FLUSH);
    return x30_zstrm.total_out;
}

}
