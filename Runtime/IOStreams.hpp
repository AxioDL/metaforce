#ifndef __PSHAG_IOSTREAMS_HPP__
#define __PSHAG_IOSTREAMS_HPP__

#include "RetroTypes.hpp"
#include <athena/IStreamReader.hpp>
#include <athena/IStreamWriter.hpp>
#include <athena/MemoryReader.hpp>
#include <zlib.h>

namespace urde
{
using CInputStream = athena::io::IStreamReader;
using COutputStream = athena::io::IStreamWriter;

struct CBitStreamReader : athena::io::MemoryCopyReader
{
    u32 x1c_val = 0;
    u32 x20_bitOffset = 0;
public:
    static s32 GetBitCount(s32 unk)
    {
        s32 ret = 0;
        while (unk != 0)
        {
            unk /= 2;
            ret++;
        }

        return ret;
    }

    CBitStreamReader(const void* data, atUint64 length)
        : MemoryCopyReader(data, length)
    {
    }

    CBitStreamReader(const std::string& filename)
        : MemoryCopyReader(filename)
    {
    }

    atUint64 readUBytesToBuf(void *buf, atUint64 len)
    {
        x20_bitOffset = 0;
        atUint64 tmp = MemoryCopyReader::readUBytesToBuf(buf, len);
        return tmp;
    }

    s32 ReadEncoded(u32 key);
};

using CMemoryInStream = athena::io::MemoryReader;

class CZipInputStream : public CInputStream
{
    std::unique_ptr<u8[]> x24_compBuf;
    std::unique_ptr<CInputStream> x28_strm;
    z_stream x30_zstrm = {};
public:
    CZipInputStream(std::unique_ptr<CInputStream>&& strm);
    ~CZipInputStream();
    atUint64 readUBytesToBuf(void *buf, atUint64 len);
    void seek(atInt64, athena::SeekOrigin) {}
    atUint64 position() const {return 0;}
    atUint64 length() const {return 0;}
};

}

#endif // __PSHAG_IOSTREAMS_HPP__
