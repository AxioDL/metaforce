#pragma once

#include "DataSpec/DNACommon/DNACommon.hpp"
#include "DNAMP1.hpp"

namespace DataSpec::DNAMP1
{

class AGSC
{
public:
    struct Header : BigDNA
    {
        AT_DECL_DNA
        String<-1> audioDir;
        String<-1> groupName;
    };
    static bool Extract(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);
    static bool Cook(const hecl::ProjectPath& inPath, const hecl::ProjectPath& outPath);
};

}

