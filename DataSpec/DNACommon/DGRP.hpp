#ifndef __COMMON_DGRP_HPP__
#define __COMMON_DGRP_HPP__

#include "DNACommon.hpp"
#include "PAK.hpp"

namespace DataSpec::DNADGRP
{

template <class IDType>
struct AT_SPECIALIZE_PARMS(DataSpec::UniqueID32, DataSpec::UniqueID64) DGRP : BigDNA
{
    AT_DECL_DNA_YAML
    Value<atUint32> dependCount;
    struct ObjectTag : BigDNA
    {
        AT_DECL_DNA_YAML
        DNAFourCC type;
        Value<IDType> id;
    };

    Vector<ObjectTag, DNA_COUNT(dependCount)> depends;
};

template <class IDType>
bool ExtractDGRP(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);
template <class IDType>
bool WriteDGRP(const DGRP<IDType>& dgrp, const hecl::ProjectPath& outPath);

}

#endif // __COMMON_DGRP_HPP__
