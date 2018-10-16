#pragma once

#include "DataSpec/DNACommon/DNACommon.hpp"
#include "../DNAMP2/CINF.hpp"

namespace DataSpec::DNAMP3
{

struct CINF : DNAMP2::CINF
{
    Delete expl;
    void sendCINFToBlender(hecl::blender::PyOutStream& os, const UniqueID64& cinfId) const;
    static std::string GetCINFArmatureName(const UniqueID64& cinfId);
};

}

