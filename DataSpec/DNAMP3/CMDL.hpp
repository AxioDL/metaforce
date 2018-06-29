#ifndef _DNAMP3_CMDL_HPP_
#define _DNAMP3_CMDL_HPP_

#include "DataSpec/DNACommon/DNACommon.hpp"
#include "DataSpec/DNACommon/CMDL.hpp"
#include "CMDLMaterials.hpp"
#include "DNAMP3.hpp"
#include "CINF.hpp"
#include "CSKR.hpp"

namespace DataSpec::DNAMP3
{

struct CMDL
{
    static bool Extract(const SpecBase& dataSpec,
                        PAKEntryReadStream& rs,
                        const hecl::ProjectPath& outPath,
                        PAKRouter<PAKBridge>& pakRouter,
                        const PAK::Entry& entry,
                        bool,
                        hecl::blender::Token& btok,
                        std::function<void(const hecl::SystemChar*)>);
};

}

#endif // _DNAMP3_CMDL_HPP_
