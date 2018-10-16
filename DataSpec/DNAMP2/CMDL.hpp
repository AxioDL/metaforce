#pragma once

#include "DataSpec/DNACommon/DNACommon.hpp"
#include "DataSpec/DNACommon/CMDL.hpp"
#include "CMDLMaterials.hpp"
#include "DNAMP2.hpp"
#include "CINF.hpp"
#include "CSKR.hpp"

namespace DataSpec::DNAMP2
{

struct CMDL
{
    static bool Extract(const SpecBase& dataSpec,
                        PAKEntryReadStream& rs,
                        const hecl::ProjectPath& outPath,
                        PAKRouter<PAKBridge>& pakRouter,
                        const DNAMP2::PAK::Entry& entry,
                        bool,
                        hecl::blender::Token& btok,
                        std::function<void(const hecl::SystemChar*)>);
};

}

