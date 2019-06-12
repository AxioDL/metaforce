#include "MAPA.hpp"
#include "../DNAMP1/DNAMP1.hpp"
#include "../DNAMP2/DNAMP2.hpp"
#include "../DNAMP3/DNAMP3.hpp"
#include "../DNAMP1/MAPA.hpp"
#include "../DNAMP2/MAPA.hpp"
#include "../DNAMP3/MAPA.hpp"
#include "zeus/CTransform.hpp"
#include "zeus/CAABox.hpp"
#include "hecl/Blender/Connection.hpp"

namespace DataSpec::DNAMAPA {

static logvisor::Module Log("DNAMAPA");

template <>
void MAPA::Enumerate<BigDNA::Read>(typename Read::StreamT& __dna_reader) {
  /* magic */
  magic = __dna_reader.readUint32Big();
  if (magic != 0xDEADD00D) {
    LogDNACommon.report(logvisor::Error, "invalid MAPA magic");
    return;
  }
  /* version */
  version = __dna_reader.readUint32Big();
  if (version == 2)
    header.reset(new HeaderMP1);
  else if (version == 3)
    header.reset(new HeaderMP2);
  else if (version == 5)
    header.reset(new HeaderMP3);
  else {
    LogDNACommon.report(logvisor::Error, "invalid MAPA version");
    return;
  }

  header->read(__dna_reader);

  for (atUint32 i = 0; i < header->mappableObjectCount(); i++) {
    std::unique_ptr<IMappableObject> mo = nullptr;
    if (version != 5)
      mo.reset(new MappableObjectMP1_2);
    else
      mo.reset(new MappableObjectMP3);
    mo->read(__dna_reader);
    mappableObjects.push_back(std::move(mo));
  }

  /* vertices */
  __dna_reader.enumerateBig(vertices, header->vertexCount());
  /* surfaceHeaders */
  __dna_reader.enumerate(surfaceHeaders, header->surfaceCount());
  /* surfaces */
  __dna_reader.enumerate(surfaces, header->surfaceCount());
}

template <>
void MAPA::Enumerate<BigDNA::Write>(typename Write::StreamT& __dna_writer) {
  /* magic */
  __dna_writer.writeUint32Big(magic);
  /* version */
  __dna_writer.writeUint32Big(version);
  header->write(__dna_writer);

  /* mappableObjects */
  for (const std::unique_ptr<IMappableObject>& mo : mappableObjects)
    mo->write(__dna_writer);
  /* vertices */
  __dna_writer.enumerateBig(vertices);
  /* surfaceHeaders */
  __dna_writer.enumerate(surfaceHeaders);
  /* surfaces */
  __dna_writer.enumerate(surfaces);
}

template <>
void MAPA::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& s) {
  header->binarySize(s);

  for (const std::unique_ptr<IMappableObject>& mo : mappableObjects)
    mo->binarySize(s);

  s += vertices.size() * 12;
  for (const SurfaceHeader& sh : surfaceHeaders)
    sh.binarySize(s);
  for (const Surface& su : surfaces)
    su.binarySize(s);
  s += 8;
}

static const char* RetroMapVisModes[] = {"ALWAYS", "MAPSTATIONORVISIT", "VISIT", "NEVER"};

static const char* RetroMapObjVisModes[] = {"ALWAYS", "MAPSTATIONORVISIT", "VISIT", "NEVER", "MAPSTATIONORVISIT2"};

template <typename PAKRouter>
bool ReadMAPAToBlender(hecl::blender::Connection& conn, const MAPA& mapa, const hecl::ProjectPath& outPath,
                       PAKRouter& pakRouter, const typename PAKRouter::EntryType& entry, bool force) {
  if (!force && outPath.isFile())
    return true;

  if (!conn.createBlend(outPath, hecl::blender::BlendType::MapArea))
    return false;
  hecl::blender::PyOutStream os = conn.beginPythonOut(true);

  os << "import bpy, bmesh\n"
        "from mathutils import Matrix\n"
        "\n"
        "bpy.types.Object.retro_mappable_type = bpy.props.IntProperty(name='Retro: MAPA object type', default=-1)\n"
        "bpy.types.Object.retro_mappable_sclyid = bpy.props.StringProperty(name='Retro: MAPA object SCLY ID')\n"
        "bpy.types.Scene.retro_map_vis_mode = bpy.props.EnumProperty(items=[('ALWAYS', 'Always', 'Always Visible', 0),"
        "('MAPSTATIONORVISIT', 'Map Station or Visit', 'Visible after Map Station or Visit', 1),"
        "('VISIT', 'Visit', 'Visible after Visit', 2),"
        "('NEVER', 'Never', 'Never Visible', 3)],"
        "name='Retro: Map Visibility Mode')\n"
        "bpy.types.Object.retro_mapobj_vis_mode = bpy.props.EnumProperty(items=[('ALWAYS', 'Always', 'Always Visible', "
        "0),"
        "('MAPSTATIONORVISIT', 'Map Station or Visit', 'Visible after Map Station or Visit', 1),"
        "('VISIT', 'Visit', 'Visible after Door Visit', 2),"
        "('NEVER', 'Never', 'Never Visible', 3),"
        "('MAPSTATIONORVISIT2', 'Map Station or Visit 2', 'Visible after Map Station or Visit', 4)],"
        "name='Retro: Map Object Visibility Mode')\n"
        "\n"
        "# Clear Scene\n"
        "if len(bpy.data.collections):\n"
        "    bpy.data.collections.remove(bpy.data.collections[0])\n"
        "\n"
        "def add_triangle(bm, verts):\n"
        "    verts = [bm.verts[vi] for vi in verts]\n"
        "    face = bm.faces.get(verts)\n"
        "    if face:\n"
        "        face = face.copy()\n"
        "        bm.verts.ensure_lookup_table()\n"
        "        face.normal_flip()\n"
        "    else:\n"
        "        bm.faces.new(verts)\n"
        "\n"
        "def add_border(bm, verts):\n"
        "    verts = [bm.verts[vi] for vi in verts]\n"
        "    edge = bm.edges.get(verts)\n"
        "    if not edge:\n"
        "        edge = bm.edges.new(verts)\n"
        "    edge.seam = True\n"
        "\n";

  os.format(
      "bpy.context.scene.name = 'MAPA_%s'\n"
      "bpy.context.scene.retro_map_vis_mode = '%s'\n",
      entry.id.toString().c_str(), RetroMapVisModes[mapa.header->visMode()]);

  /* Add empties representing MappableObjects */
  int moIdx = 0;
  for (const std::unique_ptr<MAPA::IMappableObject>& mo : mapa.mappableObjects) {
    if (mapa.version < 5) {
      const MAPA::MappableObjectMP1_2* moMP12 = static_cast<const MAPA::MappableObjectMP1_2*>(mo.get());
      zeus::simd_floats mtxF[3];
      for (int i = 0; i < 3; ++i)
        moMP12->transformMtx[i].simd.copy_to(mtxF[i]);
      os.format(
          "obj = bpy.data.objects.new('MAPOBJ_%02d', None)\n"
          "bpy.context.scene.collection.objects.link(obj)\n"
          "obj.retro_mappable_type = %d\n"
          "obj.retro_mapobj_vis_mode = '%s'\n"
          "obj.retro_mappable_sclyid = '0x%08X'\n"
          "mtx = Matrix(((%f,%f,%f,%f),(%f,%f,%f,%f),(%f,%f,%f,%f),(0.0,0.0,0.0,1.0)))\n"
          "mtxd = mtx.decompose()\n"
          "obj.rotation_mode = 'QUATERNION'\n"
          "obj.location = mtxd[0]\n"
          "obj.rotation_quaternion = mtxd[1]\n"
          "obj.scale = mtxd[2]\n",
          moIdx, int(moMP12->type), RetroMapObjVisModes[moMP12->visMode], moMP12->sclyId, mtxF[0][0], mtxF[0][1],
          mtxF[0][2], mtxF[0][3], mtxF[1][0], mtxF[1][1], mtxF[1][2], mtxF[1][3], mtxF[2][0], mtxF[2][1], mtxF[2][2],
          mtxF[2][3]);
      ++moIdx;
      continue;
    } else {
      const MAPA::MappableObjectMP3* moMP3 = static_cast<const MAPA::MappableObjectMP3*>(mo.get());
      zeus::simd_floats mtxF[3];
      for (int i = 0; i < 3; ++i)
        moMP3->transformMtx[i].simd.copy_to(mtxF[i]);
      os.format(
          "obj = bpy.data.objects.new('MAPOBJ_%02d', None)\n"
          "bpy.context.scene.collection.objects.link(obj)\n"
          "obj.retro_mappable_type = %d\n"
          "obj.retro_mapobj_vis_mode = '%s'\n"
          "obj.retro_mappable_sclyid = '0x%08X'\n"
          "mtx = Matrix(((%f,%f,%f,%f),(%f,%f,%f,%f),(%f,%f,%f,%f),(0.0,0.0,0.0,1.0)))\n"
          "mtxd = mtx.decompose()\n"
          "obj.rotation_mode = 'QUATERNION'\n"
          "obj.location = mtxd[0]\n"
          "obj.rotation_quaternion = mtxd[1]\n"
          "obj.scale = mtxd[2]\n",
          moIdx, int(moMP3->type), RetroMapObjVisModes[moMP3->visMode], moMP3->sclyId, mtxF[0][0], mtxF[0][1],
          mtxF[0][2], mtxF[0][3], mtxF[1][0], mtxF[1][1], mtxF[1][2], mtxF[1][3], mtxF[2][0], mtxF[2][1], mtxF[2][2],
          mtxF[2][3]);
      ++moIdx;
      continue;
    }
  }

  os << "# Begin bmesh\n"
        "bm = bmesh.new()\n"
        "\n";

  /* Read in verts */
  for (const atVec3f& vert : mapa.vertices) {
    zeus::simd_floats f(vert.simd);
    os.format("bm.verts.new((%f,%f,%f))\n", f[0], f[1], f[2]);
  }
  os << "bm.verts.ensure_lookup_table()\n";

  /* Read in surfaces */
  for (const typename MAPA::Surface& surf : mapa.surfaces) {
    for (const typename MAPA::Surface::Primitive& prim : surf.primitives) {
      auto iit = prim.indices.cbegin();

      /* 3 Prim Verts to start */
      int c = 0;
      unsigned int primVerts[3] = {*iit++, *iit++, *iit++};

      if (GX::Primitive(prim.type) == GX::TRIANGLESTRIP) {
        atUint8 flip = 0;
        for (size_t v = 0; v < prim.indexCount - 2; ++v) {
          if (flip) {
            os.format("add_triangle(bm, (%u,%u,%u))\n", primVerts[c % 3], primVerts[(c + 2) % 3],
                      primVerts[(c + 1) % 3]);
          } else {
            os.format("add_triangle(bm, (%u,%u,%u))\n", primVerts[c % 3], primVerts[(c + 1) % 3],
                      primVerts[(c + 2) % 3]);
          }
          flip ^= 1;

          /* Break if done */
          if (iit == prim.indices.cend())
            break;

          bool peek = (v >= prim.indexCount - 3);

          /* Advance one prim vert */
          if (peek)
            primVerts[c % 3] = *iit;
          else
            primVerts[c % 3] = *iit++;
          ++c;
        }
      } else if (GX::Primitive(prim.type) == GX::TRIANGLES) {
        for (size_t v = 0; v < prim.indexCount; v += 3) {
          os.format("add_triangle(bm, (%u,%u,%u))\n", primVerts[0], primVerts[1], primVerts[2]);

          /* Break if done */
          if (v + 3 >= prim.indexCount)
            break;

          /* Advance 3 Prim Verts */
          for (int pv = 0; pv < 3; ++pv)
            primVerts[pv] = *iit++;
        }
      }
    }

    for (const typename MAPA::Surface::Border& border : surf.borders) {
      auto iit = border.indices.cbegin();
      for (size_t i = 0; i < border.indexCount - 1; ++i) {
        os.format("add_border(bm, (%u,%u))\n", *iit, *(iit + 1));
        ++iit;
      }
    }
  }

  os << "mesh = bpy.data.meshes.new('MAP')\n"
        "obj = bpy.data.objects.new(mesh.name, mesh)\n"
        "bm.to_mesh(mesh)\n"
        "bpy.context.scene.collection.objects.link(obj)\n"
        "bm.free()\n";

  const zeus::CMatrix4f* tmpMtx = pakRouter.lookupMAPATransform(entry.id);
  const zeus::CMatrix4f& mtx = tmpMtx ? *tmpMtx : zeus::skIdentityMatrix4f;
  os.format(
      "mtx = Matrix(((%f,%f,%f,%f),(%f,%f,%f,%f),(%f,%f,%f,%f),(0.0,0.0,0.0,1.0)))\n"
      "mtxd = mtx.decompose()\n"
      "obj.rotation_mode = 'QUATERNION'\n"
      "obj.location = mtxd[0]\n"
      "obj.rotation_quaternion = mtxd[1]\n"
      "obj.scale = mtxd[2]\n",
      mtx[0][0], mtx[1][0], mtx[2][0], mtx[3][0], mtx[0][1], mtx[1][1], mtx[2][1], mtx[3][1], mtx[0][2], mtx[1][2],
      mtx[2][2], mtx[3][2]);

  /* World background */
  hecl::ProjectPath worldBlend(outPath.getParentPath().getParentPath(), "!world.blend");
  if (worldBlend.isFile())
    os.linkBackground("//../!world.blend", "World");

  os.centerView();
  os.close();
  conn.saveBlend();
  return true;
}

template bool ReadMAPAToBlender<PAKRouter<DNAMP1::PAKBridge>>(hecl::blender::Connection& conn, const MAPA& mapa,
                                                              const hecl::ProjectPath& outPath,
                                                              PAKRouter<DNAMP1::PAKBridge>& pakRouter,
                                                              const PAKRouter<DNAMP1::PAKBridge>::EntryType& entry,
                                                              bool force);

template bool ReadMAPAToBlender<PAKRouter<DNAMP2::PAKBridge>>(hecl::blender::Connection& conn, const MAPA& mapa,
                                                              const hecl::ProjectPath& outPath,
                                                              PAKRouter<DNAMP2::PAKBridge>& pakRouter,
                                                              const PAKRouter<DNAMP2::PAKBridge>::EntryType& entry,
                                                              bool force);

template bool ReadMAPAToBlender<PAKRouter<DNAMP3::PAKBridge>>(hecl::blender::Connection& conn, const MAPA& mapa,
                                                              const hecl::ProjectPath& outPath,
                                                              PAKRouter<DNAMP3::PAKBridge>& pakRouter,
                                                              const PAKRouter<DNAMP3::PAKBridge>::EntryType& entry,
                                                              bool force);

template <typename MAPAType>
bool Cook(const hecl::blender::MapArea& mapaIn, const hecl::ProjectPath& out) {
  if (mapaIn.verts.size() >= 256) {
    Log.report(logvisor::Error, _SYS_STR("MAPA %s vertex range exceeded [%d/%d]"), out.getRelativePath().data(),
               mapaIn.verts.size(), 255);
    return false;
  }

  MAPAType mapa;
  mapa.magic = 0xDEADD00D;
  mapa.version = MAPAType::Version();

  zeus::CAABox aabb;
  for (const hecl::blender::Vector3f& vert : mapaIn.verts)
    aabb.accumulateBounds(vert.val);

  mapa.header = std::make_unique<typename MAPAType::Header>();
  typename MAPAType::Header& header = static_cast<typename MAPAType::Header&>(*mapa.header);
  header.unknown1 = 0;
  header.mapVisMode = mapaIn.visType.val;
  header.boundingBox[0] = aabb.min;
  header.boundingBox[1] = aabb.max;
  header.moCount = mapaIn.pois.size();
  header.vtxCount = mapaIn.verts.size();
  header.surfCount = mapaIn.surfaces.size();

  mapa.mappableObjects.reserve(mapaIn.pois.size());
  for (const hecl::blender::MapArea::POI& poi : mapaIn.pois) {
    mapa.mappableObjects.push_back(std::make_unique<typename MAPAType::MappableObject>());
    typename MAPAType::MappableObject& mobj =
        static_cast<typename MAPAType::MappableObject&>(*mapa.mappableObjects.back());
    mobj.type = MAPA::IMappableObject::Type(poi.type);
    mobj.visMode = poi.visMode;
    mobj.sclyId = poi.objid;
    mobj.transformMtx[0] = poi.xf.val[0];
    mobj.transformMtx[1] = poi.xf.val[1];
    mobj.transformMtx[2] = poi.xf.val[2];
  }

  mapa.vertices.reserve(mapaIn.verts.size());
  for (const hecl::blender::Vector3f& vert : mapaIn.verts)
    mapa.vertices.push_back(vert.val);

  size_t offsetCur = 0;
  for (const auto& mo : mapa.mappableObjects)
    mo->binarySize(offsetCur);
  offsetCur += mapa.vertices.size() * 12;
  offsetCur += mapaIn.surfaces.size() * 32;

  mapa.surfaceHeaders.reserve(mapaIn.surfaces.size());
  mapa.surfaces.reserve(mapaIn.surfaces.size());
  for (const hecl::blender::MapArea::Surface& surfIn : mapaIn.surfaces) {
    mapa.surfaceHeaders.emplace_back();
    DNAMAPA::MAPA::SurfaceHeader& surfHead = mapa.surfaceHeaders.back();
    mapa.surfaces.emplace_back();
    DNAMAPA::MAPA::Surface& surf = mapa.surfaces.back();

    surf.primitiveCount = 1;
    surf.primitives.emplace_back();
    DNAMAPA::MAPA::Surface::Primitive& prim = surf.primitives.back();
    prim.type = GX::TRIANGLESTRIP;
    prim.indexCount = surfIn.count;
    prim.indices.reserve(surfIn.count);
    auto itBegin = mapaIn.indices.begin() + surfIn.start.val;
    auto itEnd = itBegin + surfIn.count;
    for (auto it = itBegin; it != itEnd; ++it)
      prim.indices.push_back(it->val);

    surf.borderCount = surfIn.borders.size();
    surf.borders.reserve(surfIn.borders.size());
    for (const auto& borderIn : surfIn.borders) {
      surf.borders.emplace_back();
      DNAMAPA::MAPA::Surface::Border& border = surf.borders.back();
      border.indexCount = borderIn.second.val;
      border.indices.reserve(borderIn.second.val);
      auto it2Begin = mapaIn.indices.begin() + borderIn.first.val;
      auto it2End = it2Begin + borderIn.second.val;
      for (auto it = it2Begin; it != it2End; ++it)
        border.indices.push_back(it->val);
    }

    surfHead.normal = surfIn.normal.val;
    surfHead.centroid = surfIn.centerOfMass;
    surfHead.polyOff = offsetCur;
    offsetCur += 4;
    prim.binarySize(offsetCur);
    surfHead.edgeOff = offsetCur;
    offsetCur += 4;
    for (const auto& border : surf.borders)
      border.binarySize(offsetCur);
  }

  athena::io::FileWriter f(out.getAbsolutePath());
  mapa.write(f);
  int64_t rem = f.position() % 32;
  if (rem)
    for (int64_t i = 0; i < 32 - rem; ++i)
      f.writeBytes((atInt8*)"\xff", 1);
  return true;
}

template bool Cook<DNAMP1::MAPA>(const hecl::blender::MapArea& mapa, const hecl::ProjectPath& out);
template bool Cook<DNAMP2::MAPA>(const hecl::blender::MapArea& mapa, const hecl::ProjectPath& out);
template bool Cook<DNAMP3::MAPA>(const hecl::blender::MapArea& mapa, const hecl::ProjectPath& out);

} // namespace DataSpec::DNAMAPA
