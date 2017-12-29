#ifndef _DNAMP2_CMDL_HPP_
#define _DNAMP2_CMDL_HPP_

#include "../DNACommon/DNACommon.hpp"
#include "../DNACommon/CMDL.hpp"
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
                        const DNAMP1::PAK::Entry& entry,
                        bool,
                        hecl::blender::Token& btok,
                        std::function<void(const hecl::SystemChar*)>);
};

}

#endif // _DNAMP2_CMDL_HPP_
