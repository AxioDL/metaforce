#include "CMDL.hpp"
#include "hecl/Blender/Connection.hpp"

namespace DataSpec::DNAMP3 {

bool CMDL::Extract(const SpecBase& dataSpec, PAKEntryReadStream& rs, const hecl::ProjectPath& outPath,
                   PAKRouter<PAKBridge>& pakRouter, const PAK::Entry& entry, bool, hecl::blender::Token& btok,
                   std::function<void(const hecl::SystemChar*)>) {
  /* Check for RigPair */
  CINF cinf;
  CSKR cskr;
  using RigPair = std::pair<std::pair<UniqueID64, CSKR*>, std::pair<UniqueID64, CINF*>>;
  RigPair loadRp = {};
  if (const typename CharacterAssociations<UniqueID64>::RigPair* rp = pakRouter.lookupCMDLRigPair(entry.id)) {
    pakRouter.lookupAndReadDNA(rp->cskr, cskr);
    pakRouter.lookupAndReadDNA(rp->cinf, cinf);
    loadRp.first = {rp->cskr, &cskr};
    loadRp.second = {rp->cinf, &cinf};
  }

  /* Do extract */
  hecl::blender::Connection& conn = btok.getBlenderConnection();
  if (!conn.createBlend(outPath, hecl::blender::BlendType::Mesh))
    return false;
  DNACMDL::ReadCMDLToBlender<PAKRouter<PAKBridge>, MaterialSet, RigPair, DNACMDL::SurfaceHeader_3, 5>(
      conn, rs, pakRouter, entry, dataSpec, loadRp);
  return conn.saveBlend();
}

} // namespace DataSpec::DNAMP3
