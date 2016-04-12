#ifndef __COMMON_DGRP_HPP__
#define __COMMON_DGRP_HPP__

#include "DNACommon.hpp"
#include "PAK.hpp"

namespace DataSpec
{
namespace DNADGRP
{
template <class IDType>
struct DGRP : BigYAML
{
    DECL_YAML
    Value<atUint32> dependCount;
    struct ObjectTag : BigYAML
    {
        DECL_YAML
        DNAFourCC type;
        UniqueID32 id;
    };

    Vector<ObjectTag, DNA_COUNT(dependCount)> depends;
};


template <class IDType>
bool ExtractDGRP(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);
template <class IDType>
bool WriteDGRP(const DGRP<IDType>& dgrp, const hecl::ProjectPath& outPath);
}
}
#endif // __COMMON_DGRP_HPP__
