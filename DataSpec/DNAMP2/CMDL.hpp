#ifndef _DNAMP2_CMDL_HPP_
#define _DNAMP2_CMDL_HPP_

#include "../DNACommon/DNACommon.hpp"
#include "../DNACommon/CMDL.hpp"
#include "CMDLMaterials.hpp"
#include "DNAMP2.hpp"

namespace Retro
{
namespace DNAMP2
{

struct CMDL
{
    static bool Extract(const SpecBase& dataSpec,
                        PAKEntryReadStream& rs,
                        const HECL::ProjectPath& outPath,
                        PAKRouter<PAKBridge>& pakRouter,
                        const DNAMP1::PAK::Entry& entry,
                        bool force)
    {
        HECL::BlenderConnection& conn = HECL::BlenderConnection::SharedConnection();
        if (!conn.createBlend(outPath.getAbsolutePath()))
            return false;
        DNACMDL::ReadCMDLToBlender<PAKRouter<PAKBridge>, MaterialSet, 4>
                (conn, rs, pakRouter, entry, dataSpec.getMasterShaderPath());
        return conn.saveBlend();
    }
};

}
}

#endif // _DNAMP2_CMDL_HPP_
