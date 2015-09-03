#ifndef _DNAMP2_CMDL_HPP_
#define _DNAMP2_CMDL_HPP_

#include "../DNACommon/DNACommon.hpp"
#include "../DNACommon/CMDL.hpp"
#include "CMDLMaterials.hpp"
#include "DNAMP2.hpp"
#include "CINF.hpp"
#include "CSKR.hpp"

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
                        bool force,
                        std::function<void(const HECL::SystemChar*)> fileChanged)
    {
        HECL::BlenderConnection& conn = HECL::BlenderConnection::SharedConnection();
        if (!conn.createBlend(outPath.getAbsolutePath()))
            return false;
        DNACMDL::ReadCMDLToBlender<PAKRouter<PAKBridge>, MaterialSet, std::pair<CSKR*,CINF*>, 4>
                (conn, rs, pakRouter, entry, dataSpec);
        return conn.saveBlend();
    }
};

}
}

#endif // _DNAMP2_CMDL_HPP_
