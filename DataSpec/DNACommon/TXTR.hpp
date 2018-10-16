#pragma once

#include "DNACommon.hpp"

namespace DataSpec
{
class PAKEntryReadStream;

struct TXTR
{
    static bool Extract(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);
    static bool Cook(const hecl::ProjectPath& inPath, const hecl::ProjectPath& outPath);
    static bool CookPC(const hecl::ProjectPath& inPath, const hecl::ProjectPath& outPath);
};

}

