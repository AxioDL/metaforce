#ifndef _DNAMP1_AGSC_HPP_
#define _DNAMP1_AGSC_HPP_

#include "../DNACommon/DNACommon.hpp"
#include "DNAMP1.hpp"

namespace DataSpec::DNAMP1
{

class AGSC
{
    struct Header : BigDNA
    {
        DECL_DNA
        String<-1> audioDir;
        String<-1> groupName;
    };
public:
    static bool Extract(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);
    static bool Cook(const hecl::ProjectPath& inPath, const hecl::ProjectPath& outPath);
};

}

#endif // _DNAMP1_AGSC_HPP_
