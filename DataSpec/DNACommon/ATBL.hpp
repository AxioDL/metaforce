#pragma once

#include "DNACommon.hpp"
#include "PAK.hpp"

namespace DataSpec::DNAAudio
{

class ATBL
{
public:
    static bool Extract(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);
    static bool Cook(const hecl::ProjectPath& inPath, const hecl::ProjectPath& outPath);
};

}

