#ifndef RETROCOMMON_HPP
#define RETROCOMMON_HPP

#include <Athena/IStreamWriter.hpp>
#include <Athena/IStreamReader.hpp>

namespace aIO = Athena::io;


enum class EPAKSection
{
    STRG = 0x53545247,
    RSHD = 0x52534844,
    DATA = 0x44415441
};

void decompressData(aIO::IStreamWriter& outbuf,  const atUint8* srcData, atUint32 srcLength, atInt32 uncompressedLength);
void decompressFile(aIO::IStreamWriter& outbuf,  const atUint8* srcData, atUint32 srcLength);
bool decompressMREA(aIO::IStreamReader& in, aIO::IStreamWriter& out);

#endif // RETROCOMMON_HPP
