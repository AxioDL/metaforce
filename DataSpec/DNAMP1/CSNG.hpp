#pragma once

#include "DataSpec/DNACommon/DNACommon.hpp"
#include "DNAMP1.hpp"

namespace DataSpec::DNAMP1
{

class CSNG
{
    struct Header : BigDNA
    {
        AT_DECL_DNA
        Value<atUint32> magic = 0x2;
        Value<atUint32> midiSetupId;
        Value<atUint32> songGroupId;
        UniqueID32 agscId;
        Value<atUint32> sngLength;
    };
public:
    static bool Extract(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);
    static bool Cook(const hecl::ProjectPath& inPath, const hecl::ProjectPath& outPath);
};

}

