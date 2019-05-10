#include <athena/FileWriter.hpp>
#include "MREA.hpp"
#include "../DNAMP2/DeafBabe.hpp"
#include "DataSpec/DNACommon/BabeDead.hpp"
#include "hecl/Blender/Connection.hpp"

namespace DataSpec {
extern hecl::Database::DataSpecEntry SpecEntMP3ORIG;

namespace DNAMP3 {

MREA::StreamReader::StreamReader(athena::io::IStreamReader& source, atUint32 blkCount, atUint32 secIdxCount)
: DNAMP2::MREA::StreamReader(source) {
  m_blkCount = blkCount;
  m_blockInfos.reserve(blkCount);
  for (atUint32 i = 0; i < blkCount; ++i) {
    m_blockInfos.emplace_back();
    BlockInfo& info = m_blockInfos.back();
    info.read(source);
    m_totalDecompLen += info.decompSize;
  }
  source.seekAlign32();
  m_secIdxs.reserve(secIdxCount);
  for (atUint32 i = 0; i < secIdxCount; ++i) {
    m_secIdxs.emplace_back();
    std::pair<DNAFourCC, atUint32>& idx = m_secIdxs.back();
    idx.first.read(source);
    idx.second = source.readUint32Big();
  }
  source.seekAlign32();
  m_blkBase = source.position();
  nextBlock();
}

void MREA::StreamReader::writeSecIdxs(athena::io::IStreamWriter& writer) const {
  for (const std::pair<DNAFourCC, atUint32>& idx : m_secIdxs) {
    idx.first.write(writer);
    writer.writeUint32Big(idx.second);
  }
}

void MREA::ReadBabeDeadToBlender_3(hecl::blender::PyOutStream& os, athena::io::IStreamReader& rs) {
  atUint32 bdMagic = rs.readUint32Big();
  if (bdMagic != 0xBABEDEAD)
    Log.report(logvisor::Fatal, "invalid BABEDEAD magic");
  os << "bpy.context.scene.world.use_nodes = True\n"
        "bg_node = bpy.context.scene.world.node_tree.nodes['Background']\n"
        "bg_node.inputs[1].default_value = 0.0\n";
  for (atUint32 s = 0; s < 4; ++s) {
    atUint32 lightCount = rs.readUint32Big();
    for (atUint32 l = 0; l < lightCount; ++l) {
      BabeDeadLight light;
      light.read(rs);
      ReadBabeDeadLightToBlender(os, light, s, l);
    }
  }
}

bool MREA::Extract(const SpecBase& dataSpec, PAKEntryReadStream& rs, const hecl::ProjectPath& outPath,
                   PAKRouter<PAKBridge>& pakRouter, const PAK::Entry& entry, bool force, hecl::blender::Token& btok,
                   std::function<void(const hecl::SystemChar*)>) {
  using RigPair = std::pair<CSKR*, CINF*>;
  RigPair dummy(nullptr, nullptr);

  if (!force && outPath.isFile())
    return true;

  /* Do extract */
  Header head;
  head.read(rs);
  rs.seekAlign32();

  /* MREA decompression stream */
  StreamReader drs(rs, head.compressedBlockCount, head.secIndexCount);
  hecl::ProjectPath decompPath = outPath.getCookedPath(SpecEntMP3ORIG).getWithExtension(_SYS_STR(".decomp"));
  decompPath.makeDirChain(false);
  athena::io::FileWriter mreaDecompOut(decompPath.getAbsolutePath());
  head.write(mreaDecompOut);
  mreaDecompOut.seekAlign32();
  drs.writeDecompInfos(mreaDecompOut);
  mreaDecompOut.seekAlign32();
  drs.writeSecIdxs(mreaDecompOut);
  mreaDecompOut.seekAlign32();
  atUint64 decompLen = drs.length();
  mreaDecompOut.writeBytes(drs.readBytes(decompLen).get(), decompLen);
  mreaDecompOut.close();
  drs.seek(0, athena::Begin);

  /* Start up blender connection */
  hecl::blender::Connection& conn = btok.getBlenderConnection();
  if (!conn.createBlend(outPath, hecl::blender::BlendType::Area))
    return false;

  /* Open Py Stream and read sections */
  hecl::blender::PyOutStream os = conn.beginPythonOut(true);
  os.format(
      "import bpy\n"
      "import bmesh\n"
      "from mathutils import Vector\n"
      "\n"
      "bpy.context.scene.name = '%s'\n",
      pakRouter.getBestEntryName(entry, false).c_str());
  DNACMDL::InitGeomBlenderContext(os, dataSpec.getMasterShaderPath());
  MaterialSet::RegisterMaterialProps(os);
  os << "# Clear Scene\n"
        "if 'Collection 1' in bpy.data.collections:\n"
        "    bpy.data.collections.remove(bpy.data.collections['Collection 1'])\n"
        "\n"
        "bpy.types.Light.retro_layer = bpy.props.IntProperty(name='Retro: Light Layer')\n"
        "bpy.types.Light.retro_origtype = bpy.props.IntProperty(name='Retro: Original Type')\n"
        "\n";

  /* One shared material set for all meshes */
  os << "# Materials\n"
        "materials = []\n"
        "\n";
  MaterialSet matSet;
  atUint64 secStart = drs.position();
  matSet.read(drs);
  matSet.readToBlender(os, pakRouter, entry, 0);
  drs.seek(secStart + head.secSizes[0], athena::Begin);
  std::vector<DNACMDL::VertexAttributes> vertAttribs;
  DNACMDL::GetVertexAttributes(matSet, vertAttribs);

  /* Read mesh info */
  atUint32 curSec = 1;
  std::vector<atUint32> surfaceCounts;
  surfaceCounts.reserve(head.meshCount);
  for (atUint32 m = 0; m < head.meshCount; ++m) {
    /* Mesh header */
    MeshHeader mHeader;
    secStart = drs.position();
    mHeader.read(drs);
    drs.seek(secStart + head.secSizes[curSec++], athena::Begin);

    /* Surface count from here */
    secStart = drs.position();
    surfaceCounts.push_back(drs.readUint32Big());
    drs.seek(secStart + head.secSizes[curSec++], athena::Begin);

    /* Seek through AROT-relation sections */
    drs.seek(head.secSizes[curSec++], athena::Current);
    drs.seek(head.secSizes[curSec++], athena::Current);
  }

  /* Skip though WOBJs */
  auto secIdxIt = drs.beginSecIdxs();
  while (secIdxIt->first == FOURCC('WOBJ'))
    ++secIdxIt;

  /* Skip AROT */
  if (secIdxIt->first == FOURCC('ROCT')) {
    drs.seek(head.secSizes[curSec++], athena::Current);
    ++secIdxIt;
  }

  /* Skip AABB */
  if (secIdxIt->first == FOURCC('AABB')) {
    drs.seek(head.secSizes[curSec++], athena::Current);
    ++secIdxIt;
  }

  /* Now the meshes themselves */
  if (secIdxIt->first == FOURCC('GPUD')) {
    for (atUint32 m = 0; m < head.meshCount; ++m) {
      curSec +=
          DNACMDL::ReadGeomSectionsToBlender<PAKRouter<PAKBridge>, MaterialSet, RigPair, DNACMDL::SurfaceHeader_3>(
              os, drs, pakRouter, entry, dummy, true, false, vertAttribs, m, head.secCount, 0, &head.secSizes[curSec],
              surfaceCounts[m]);
    }
    ++secIdxIt;
  }

  /* Skip DEPS */
  if (secIdxIt->first == FOURCC('DEPS')) {
    drs.seek(head.secSizes[curSec++], athena::Current);
    ++secIdxIt;
  }

  /* Skip SOBJ (SCLY) */
  if (secIdxIt->first == FOURCC('SOBJ')) {
    for (atUint32 l = 0; l < head.sclyLayerCount; ++l)
      drs.seek(head.secSizes[curSec++], athena::Current);
    ++secIdxIt;
  }

  /* Skip SGEN */
  if (secIdxIt->first == FOURCC('SGEN')) {
    drs.seek(head.secSizes[curSec++], athena::Current);
    ++secIdxIt;
  }

  /* Read Collision Meshes */
  if (secIdxIt->first == FOURCC('COLI')) {
    DNAMP2::DeafBabe collision;
    secStart = drs.position();
    collision.read(drs);
    DNAMP2::DeafBabe::BlenderInit(os);
    collision.sendToBlender(os);
    drs.seek(secStart + head.secSizes[curSec++], athena::Begin);
    ++secIdxIt;
  }

  /* Read BABEDEAD Lights as Cycles emissives */
  if (secIdxIt->first == FOURCC('LITE')) {
    secStart = drs.position();
    ReadBabeDeadToBlender_3(os, drs);
    drs.seek(secStart + head.secSizes[curSec++], athena::Begin);
    ++secIdxIt;
  }

  /* Origins to center of mass */
  os << "bpy.context.view_layer.layer_collection.children['Collision'].hide_viewport = False\n"
        "bpy.ops.object.select_by_type(type='MESH')\n"
        "bpy.ops.object.origin_set(type='ORIGIN_CENTER_OF_MASS')\n"
        "bpy.ops.object.select_all(action='DESELECT')\n"
        "bpy.context.view_layer.layer_collection.children['Collision'].hide_viewport = True\n";

  os.centerView();
  os.close();
  return conn.saveBlend();
}

bool MREA::ExtractLayerDeps(PAKEntryReadStream& rs, PAKBridge::Level::Area& areaOut) {
  /* Do extract */
  Header head;
  head.read(rs);
  rs.seekAlign32();

  /* MREA decompression stream */
  StreamReader drs(rs, head.compressedBlockCount, head.secIndexCount);
  for (const std::pair<DNAFourCC, atUint32>& idx : drs.m_secIdxs) {
    if (idx.first == FOURCC('DEPS')) {
      drs.seek(head.getSecOffset(idx.second), athena::Begin);
      DEPS deps;
      deps.read(drs);

      unsigned r = 0;
      for (unsigned l = 1; l < deps.depLayerCount; ++l) {
        if (l > areaOut.layers.size())
          break;
        PAKBridge::Level::Area::Layer& layer = areaOut.layers.at(l - 1);
        layer.resources.reserve(deps.depLayers[l] - r);
        for (; r < deps.depLayers[l]; ++r)
          layer.resources.emplace(deps.deps[r].id);
      }
      areaOut.resources.reserve(deps.depCount - r + 2);
      for (; r < deps.depCount; ++r)
        areaOut.resources.emplace(deps.deps[r].id);

      return true;
    }
  }
  return false;
}

} // namespace DNAMP3
} // namespace DataSpec
