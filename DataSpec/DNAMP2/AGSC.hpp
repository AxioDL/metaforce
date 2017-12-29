#ifndef _DNAMP2_AGSC_HPP_
#define _DNAMP2_AGSC_HPP_

#include "../DNACommon/DNACommon.hpp"
#include "DNAMP2.hpp"

namespace DataSpec::DNAMP2
{

class AGSC
{
    struct Header : BigDNA
    {
        DECL_DNA
        Value<atUint32> unk;
        String<-1> groupName;
        Value<atUint16> groupId = -1;
        Value<atUint32> poolSz = 0;
        Value<atUint32> projSz = 0;
        Value<atUint32> sdirSz = 0;
        Value<atUint32> sampSz = 0;
    };
public:
    static bool Extract(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);
    static bool Cook(const hecl::ProjectPath& inPath, const hecl::ProjectPath& outPath);
};

}

#endif // _DNAMP2_AGSC_HPP_
