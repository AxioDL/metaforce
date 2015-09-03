#ifndef _DNAMP3_CMDL_HPP_
#define _DNAMP3_CMDL_HPP_

#include "../DNACommon/DNACommon.hpp"
#include "../DNACommon/CMDL.hpp"
#include "CMDLMaterials.hpp"
#include "DNAMP3.hpp"
#include "CINF.hpp"
#include "CSKR.hpp"

namespace Retro
{
namespace DNAMP3
{

struct CMDL
{
    static bool Extract(const SpecBase& dataSpec,
                        PAKEntryReadStream& rs,
                        const HECL::ProjectPath& outPath,
                        PAKRouter<PAKBridge>& pakRouter,
                        const PAK::Entry& entry,
                        bool force,
                        std::function<void(const HECL::SystemChar*)> fileChanged)
    {
        HECL::BlenderConnection& conn = HECL::BlenderConnection::SharedConnection();
        if (!conn.createBlend(outPath.getAbsolutePath()))
            return false;
        DNACMDL::ReadCMDLToBlender<PAKRouter<PAKBridge>, MaterialSet, std::pair<CSKR*,CINF*>, 5>
                (conn, rs, pakRouter, entry, dataSpec);
        return conn.saveBlend();
    }
};

}
}

#endif // _DNAMP3_CMDL_HPP_
