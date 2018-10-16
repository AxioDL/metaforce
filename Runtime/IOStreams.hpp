#pragma once

#include "GCNTypes.hpp"
#include <athena/IStreamReader.hpp>
#include <athena/IStreamWriter.hpp>
#include <athena/MemoryReader.hpp>
#include <athena/MemoryWriter.hpp>
#include <zlib.h>

namespace urde
{
using CInputStream = athena::io::IStreamReader;
using COutputStream = athena::io::IStreamWriter;

struct CBitStreamReader : athena::io::MemoryReader
{
    u32 x1c_val = 0;
    u32 x20_bitOffset = 0;
public:
    static u32 GetBitCount(u32 maxVal)
    {
        u32 ret = 0;
        while (maxVal != 0)
        {
            maxVal /= 2;
            ret++;
        }

        return ret;
    }

    CBitStreamReader(const void* data, atUint64 length)
    : MemoryReader(data, length) {}

    s32 ReadEncoded(u32 key);
};

class CBitStreamWriter : public athena::io::MemoryWriter
{
    u32 x14_val = 0;
    u32 x18_bitOffset = 0x20;
public:
    static inline u32 GetBitCount(u32 maxVal) { return CBitStreamReader::GetBitCount(maxVal); }

    CBitStreamWriter(atUint8* data = nullptr, atUint64 length=0x10)
    : MemoryWriter(data, length) {}

    void WriteEncoded(u32 val, u32 bitCount);

    void Flush();

    ~CBitStreamWriter() { Flush(); }
};

using CMemoryInStream = athena::io::MemoryReader;
using CMemoryOutStream = athena::io::MemoryWriter;

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

