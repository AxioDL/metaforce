#include "CMDL.hpp"
#include "hecl/Blender/Connection.hpp"

namespace DataSpec::DNAMP3
{

bool CMDL::Extract(const SpecBase& dataSpec,
                   PAKEntryReadStream& rs,
                   const hecl::ProjectPath& outPath,
                   PAKRouter<PAKBridge>& pakRouter,
                   const PAK::Entry& entry,
                   bool,
                   hecl::blender::Token& btok,
                   std::function<void(const hecl::SystemChar*)>)
{
    /* Check for RigPair */
    const typename CharacterAssociations<UniqueID64>::RigPair* rp = pakRouter.lookupCMDLRigPair(entry.id);
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
    hecl::blender::Connection& conn = btok.getBlenderConnection();
    if (!conn.createBlend(outPath, hecl::blender::BlendType::Mesh))
        return false;
    DNACMDL::ReadCMDLToBlender<PAKRouter<PAKBridge>, MaterialSet, std::pair<CSKR*,CINF*>, DNACMDL::SurfaceHeader_3, 5>
        (conn, rs, pakRouter, entry, dataSpec, loadRp);
    return conn.saveBlend();
}

}

