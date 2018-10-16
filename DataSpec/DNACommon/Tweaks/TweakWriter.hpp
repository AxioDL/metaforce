#pragma once

#include "../PAK.hpp"

namespace DataSpec
{

template <class T>
bool WriteTweak(const T& tweak, const hecl::ProjectPath& outPath)
{
    athena::io::FileWriter w(outPath.getAbsolutePath(), true, false);
    if (w.hasError())
        return false;
    tweak.write(w);
    int64_t rem = w.position() % 32;
    if (rem)
        for (int64_t i=0 ; i<32-rem ; ++i)
            w.writeUByte(0xff);
    return true;
}

template <class T>
bool ExtractTweak(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath)
{
    athena::io::FileWriter writer(outPath.getAbsolutePath());
    if (writer.isOpen())
    {
        T tweak;
        tweak.read(rs);
        athena::io::ToYAMLStream(tweak, writer);
        return true;
    }
    return false;
}

}

