#include "DataSpec/DNAMP2/PATH.hpp"
#include "hecl/Blender/Connection.hpp"

namespace DataSpec::DNAMP2 {
bool PATH::Extract(const SpecBase& dataSpec, PAKEntryReadStream& rs, const hecl::ProjectPath& outPath,
                   PAKRouter<PAKBridge>& pakRouter, const PAK::Entry& entry, bool force, hecl::blender::Token& btok,
                   std::function<void(const hecl::SystemChar*)> fileChanged) {
  PATH path;
  path.read(rs);
  hecl::blender::Connection& conn = btok.getBlenderConnection();
  if (!conn.createBlend(outPath, hecl::blender::BlendType::PathMesh))
    return false;

  std::string areaPath;
  for (const auto& ent : hecl::DirectoryEnumerator(outPath.getParentPath().getAbsolutePath())) {
    if (hecl::StringUtils::BeginsWith(ent.m_name, _SYS_STR("!area_"))) {
      areaPath = hecl::SystemUTF8Conv(ent.m_name).str();
      break;
    }
  }

  const zeus::CMatrix4f* xf = pakRouter.lookupMAPATransform(entry.id);
  path.sendToBlender(conn, pakRouter.getBestEntryName(entry, false), xf, areaPath);
  return conn.saveBlend();
}
}