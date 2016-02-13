#include "IOStreams.hpp"
#include "CMemory.hpp"

namespace pshag
{

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
