#ifndef __PSHAG_IOSTREAMS_HPP__
#define __PSHAG_IOSTREAMS_HPP__

#include "RetroTypes.hpp"
#include <Athena/IStreamReader.hpp>
#include <Athena/IStreamWriter.hpp>
#include <Athena/MemoryReader.hpp>
#include <zlib.h>

namespace pshag
{

using CInputStream = Athena::io::IStreamReader;
using COutputStream = Athena::io::IStreamWriter;

using CMemoryInStream = Athena::io::MemoryReader;

class CZipInputStream : public CInputStream
{
    std::unique_ptr<u8[]> x24_compBuf;
    std::unique_ptr<CInputStream> x28_strm;
    z_stream x30_zstrm = {};
public:
    CZipInputStream(std::unique_ptr<CInputStream>&& strm);
    ~CZipInputStream();
    atUint64 readUBytesToBuf(void *buf, atUint64 len);
    void seek(atInt64, Athena::SeekOrigin) {}
    atUint64 position() const {return 0;}
    atUint64 length() const {return 0;}
};

}

#endif // __PSHAG_IOSTREAMS_HPP__
