#include "IOStreams.hpp"
#include "CMemory.hpp"

namespace urde
{

s32 DecryptionCtx::DecryptRead(CInputStream& in, s32 key)
{
    int ret = 0;
    if (x20_encShift >= key)
    {
        int diff = x20_encShift - 0x20;
        int baseVal = -1;
        if (x20_encShift != 0x20)
            baseVal = 1 << (x20_encShift - 1);
        x20_encShift = key - x20_encShift;
        ret = baseVal | (x1c_encVal >> diff);
    }
    else
    {
        int diff = x20_encShift - key;
        int rem = x20_encShift - 0x20;
        int baseVal1 = -1;
        if (x20_encShift != 0x20)
            baseVal1 = 1 << (x20_encShift - 1);

        int bit = diff & 7;
        x20_encShift = 0;
        int count = (diff >> 3) + ((-bit | bit) >> 31);
        int baseVal2 = (baseVal1 & (x1c_encVal >> rem)) << diff;
        in.readBytesToBuf(&x1c_encVal, count);

        int baseVal3 = -1;
        if (diff != 0x20)
            baseVal3 = 1 << (diff - 1);

        int tmpShift = x20_encShift;
        x20_encShift = (count << 3);
        ret = baseVal2 | (baseVal3 & (x1c_encVal >> (diff - 0x20))) << tmpShift;
        x20_encShift = diff - x20_encShift;
        x1c_encVal = x1c_encVal >> diff;
    }

    return ret;
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
