#ifndef __DNACOMMON_TXTR_HPP__
#define __DNACOMMON_TXTR_HPP__

#include "DNACommon.hpp"

namespace Retro
{

struct TXTR
{
    static bool Extract(const SpecBase& dataspec, PAKEntryReadStream& rs, const HECL::ProjectPath& outPath);
    static bool Cook(const HECL::ProjectPath& inPath, const HECL::ProjectPath& outPath);
};

}

#endif // __DNACOMMON_TXTR_HPP__
