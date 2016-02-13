#ifndef _DNAMP3_CMDL_HPP_
#define _DNAMP3_CMDL_HPP_

#include "../DNACommon/DNACommon.hpp"
#include "../DNACommon/CMDL.hpp"
#include "CMDLMaterials.hpp"
#include "DNAMP3.hpp"
#include "CINF.hpp"
#include "CSKR.hpp"

namespace DataSpec
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
                        bool,
                        std::function<void(const HECL::SystemChar*)>)
    {
        /* Check for RigPair */
        const PAKRouter<PAKBridge>::RigPair* rp = pakRouter.lookupCMDLRigPair(entry.id);
        CINF cinf;
        CSKR cskr;
        std::pair<CSKR*,CINF*> loadRp(nullptr, nullptr);
        if (rp)
        {
            pakRouter.lookupAndReadDNA(rp->first, cskr);
            pakRouter.lookupAndReadDNA(rp->second, cinf);
            loadRp.first = &cskr;
            loadRp.second = &cinf;
        }

        /* Do extract */
        HECL::BlenderConnection& conn = HECL::BlenderConnection::SharedConnection();
        if (!conn.createBlend(outPath, HECL::BlenderConnection::BlendType::Mesh))
            return false;
        DNACMDL::ReadCMDLToBlender<PAKRouter<PAKBridge>, MaterialSet, std::pair<CSKR*,CINF*>, DNACMDL::SurfaceHeader_3, 5>
                (conn, rs, pakRouter, entry, dataSpec, loadRp);
        return conn.saveBlend();
    }
};

}
}

#endif // _DNAMP3_CMDL_HPP_
