#include "AGSC.hpp"
#include "amuse/AudioGroup.hpp"
#include "amuse/AudioGroupData.hpp"

namespace DataSpec::DNAMP2 {

bool AGSC::Extract(PAKEntryReadStream& rs, const hecl::ProjectPath& dir) {
  dir.makeDirChain(true);

  Header head;
  head.read(rs);

  auto pool = rs.readUBytes(head.poolSz);
  auto proj = rs.readUBytes(head.projSz);
  auto sdir = rs.readUBytes(head.sdirSz);
  auto samp = rs.readUBytes(head.sampSz);

  amuse::AudioGroupData data(proj.get(), head.projSz, pool.get(), head.poolSz, sdir.get(), head.sdirSz, samp.get(),
                             head.sampSz, amuse::GCNDataTag{});

  /* Load into amuse representation */
  amuse::ProjectDatabase projDb;
  projDb.setIdDatabases();
  amuse::AudioGroupDatabase group(data);
  group.setGroupPath(dir.getAbsolutePath());

  /* Extract samples */
  group.getSdir().extractAllCompressed(dir.getAbsolutePath(), data.getSamp());

  /* Write out project/pool */
  {
    auto projd = group.getProj().toYAML();
    athena::io::FileWriter fo(hecl::ProjectPath(dir, _SYS_STR("!project.yaml")).getAbsolutePath());
    if (fo.hasError())
      return false;
    fo.writeUBytes(projd.data(), projd.size());
  }

  {
    auto poold = group.getPool().toYAML();
    athena::io::FileWriter fo(hecl::ProjectPath(dir, _SYS_STR("!pool.yaml")).getAbsolutePath());
    if (fo.hasError())
      return false;
    fo.writeUBytes(poold.data(), poold.size());
  }

  return true;
}

bool AGSC::Cook(const hecl::ProjectPath& dir, const hecl::ProjectPath& outPath) {
  athena::io::FileWriter w(outPath.getAbsolutePath());
  if (w.hasError())
    return false;

  amuse::ProjectDatabase projDb;
  projDb.setIdDatabases();
  amuse::AudioGroupDatabase group(dir.getAbsolutePath());

  auto proj = group.getProj().toGCNData(group.getPool(), group.getSdir());
  auto pool = group.getPool().toData<athena::Endian::Big>();
  auto sdirSamp = group.getSdir().toGCNData(group);

  Header head;
  head.groupName = dir.getLastComponentUTF8();
  for (const auto& p : group.getProj().sfxGroups())
    head.groupId = p.first.id;
  head.poolSz = pool.size();
  head.projSz = proj.size();
  head.sdirSz = sdirSamp.first.size();
  head.sampSz = sdirSamp.second.size();
  head.write(w);

  w.writeUBytes(pool.data(), pool.size());
  w.writeUBytes(proj.data(), proj.size());
  w.writeUBytes(sdirSamp.first.data(), sdirSamp.first.size());
  w.writeUBytes(sdirSamp.second.data(), sdirSamp.second.size());

  return true;
}

} // namespace DataSpec::DNAMP2
