#ifndef __DNACOMMON_TXTR_HPP__
#define __DNACOMMON_TXTR_HPP__

#include "DNACommon.hpp"

namespace DataSpec
{
class PAKEntryReadStream;

struct TXTR
{
    static bool Extract(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);
    static bool Cook(const hecl::ProjectPath& inPath, const hecl::ProjectPath& outPath);
};

}

#endif // __DNACOMMON_TXTR_HPP__
