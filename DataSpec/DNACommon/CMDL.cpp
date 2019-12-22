#include "DataSpec/DNACommon/CMDL.hpp"

#include <utility>

#include "DataSpec/DNAMP1/CMDLMaterials.hpp"
#include "DataSpec/DNAMP1/CSKR.hpp"
#include "DataSpec/DNAMP1/MREA.hpp"
#include "DataSpec/DNAMP2/CMDLMaterials.hpp"
#include "DataSpec/DNAMP2/CSKR.hpp"
#include "DataSpec/DNAMP3/CMDLMaterials.hpp"
#include "DataSpec/DNAMP3/CSKR.hpp"

#include <fmt/format.h>
#include <hecl/Blender/Connection.hpp>
#include <zeus/CAABox.hpp>

namespace DataSpec::DNACMDL {

template <class MaterialSet>
void GetVertexAttributes(const MaterialSet& matSet, std::vector<VertexAttributes>& attributesOut) {
  attributesOut.clear();
  attributesOut.reserve(matSet.materials.size());

  for (const typename MaterialSet::Material& mat : matSet.materials) {
    const typename MaterialSet::Material::VAFlags& vaFlags = mat.getVAFlags();
    attributesOut.emplace_back();
    VertexAttributes& va = attributesOut.back();

    va.pos = vaFlags.position();
    va.norm = vaFlags.normal();
    va.color0 = vaFlags.color0();
    va.color1 = vaFlags.color1();

    if ((va.uvs[0] = vaFlags.tex0()))
      ++va.uvCount;
    if ((va.uvs[1] = vaFlags.tex1()))
      ++va.uvCount;
    if ((va.uvs[2] = vaFlags.tex2()))
      ++va.uvCount;
    if ((va.uvs[3] = vaFlags.tex3()))
      ++va.uvCount;
    if ((va.uvs[4] = vaFlags.tex4()))
      ++va.uvCount;
    if ((va.uvs[5] = vaFlags.tex5()))
      ++va.uvCount;
    if ((va.uvs[6] = vaFlags.tex6()))
      ++va.uvCount;

    va.pnMtxIdx = vaFlags.pnMatIdx();

    if ((va.texMtxIdx[0] = vaFlags.tex0MatIdx()))
      ++va.texMtxIdxCount;
    if ((va.texMtxIdx[1] = vaFlags.tex1MatIdx()))
      ++va.texMtxIdxCount;
    if ((va.texMtxIdx[2] = vaFlags.tex2MatIdx()))
      ++va.texMtxIdxCount;
    if ((va.texMtxIdx[3] = vaFlags.tex3MatIdx()))
      ++va.texMtxIdxCount;
    if ((va.texMtxIdx[4] = vaFlags.tex4MatIdx()))
      ++va.texMtxIdxCount;
    if ((va.texMtxIdx[5] = vaFlags.tex5MatIdx()))
      ++va.texMtxIdxCount;
    if ((va.texMtxIdx[6] = vaFlags.tex6MatIdx()))
      ++va.texMtxIdxCount;

    va.shortUVs = mat.getFlags().lightmapUVArray();
  }
}

template <class PAKRouter, class MaterialSet>
void ReadMaterialSetToBlender_1_2(hecl::blender::PyOutStream& os, const MaterialSet& matSet, const PAKRouter& pakRouter,
                                  const typename PAKRouter::EntryType& entry, unsigned setIdx) {
  /* Texmaps */
  os << "texmap_list = []\n";
  for (const UniqueID32& tex : matSet.head.textureIDs) {
    std::string texName = pakRouter.getBestEntryName(tex);
    const nod::Node* node;
    const typename PAKRouter::EntryType* texEntry = pakRouter.lookupEntry(tex, &node);
    hecl::ProjectPath txtrPath = pakRouter.getWorking(texEntry);
    if (!txtrPath.isNone()) {
      txtrPath.makeDirChain(false);
      PAKEntryReadStream rs = texEntry->beginReadStream(*node);
      TXTR::Extract(rs, txtrPath);
    }
    hecl::SystemString resPath = pakRouter.getResourceRelativePath(entry, tex);
    hecl::SystemUTF8Conv resPathView(resPath);
    os.format(fmt(
        "if '{}' in bpy.data.images:\n"
        "    image = bpy.data.images['{}']\n"
        "else:\n"
        "    image = bpy.data.images.load('''//{}''')\n"
        "    image.name = '{}'\n"
        "texmap_list.append(image)\n"
        "\n"),
        texName, texName, resPathView, texName);
  }

  unsigned m = 0;
  for (const typename MaterialSet::Material& mat : matSet.materials) {
    MaterialSet::ConstructMaterial(os, mat, setIdx, m++);
    os << "materials.append(new_material)\n";
  }
}

template <class PAKRouter, class MaterialSet>
void ReadMaterialSetToBlender_3(hecl::blender::PyOutStream& os, const MaterialSet& matSet, const PAKRouter& pakRouter,
                                const typename PAKRouter::EntryType& entry, unsigned setIdx) {
  unsigned m = 0;
  for (const typename MaterialSet::Material& mat : matSet.materials) {
    MaterialSet::ConstructMaterial(os, pakRouter, entry, mat, setIdx, m++);
    os << "materials.append(new_material)\n";
  }
}

template void ReadMaterialSetToBlender_3<PAKRouter<DNAMP3::PAKBridge>, DNAMP3::MaterialSet>(
    hecl::blender::PyOutStream& os, const DNAMP3::MaterialSet& matSet, const PAKRouter<DNAMP3::PAKBridge>& pakRouter,
    const PAKRouter<DNAMP3::PAKBridge>::EntryType& entry, unsigned setIdx);

class DLReader {
public:
  /* Class used for splitting verts with shared positions but different skinning matrices */
  class ExtraVertTracker {
    std::map<atUint16, std::vector<std::pair<atInt16, atUint16>>> m_extraVerts;
    atUint16 m_maxBasePos = 0;
    atUint16 m_nextOverPos = 1;

  public:
    atInt16 addPosSkinPair(atUint16 pos, atInt16 skin) {
      m_maxBasePos = std::max(m_maxBasePos, pos);
      auto search = m_extraVerts.find(pos);
      if (search == m_extraVerts.end()) {
        m_extraVerts[pos] = {std::make_pair(skin, 0)};
        return skin;
      }
      std::vector<std::pair<atInt16, atUint16>>& vertTrack = search->second;
      for (const std::pair<atInt16, atUint16>& s : vertTrack)
        if (s.first == skin)
          return vertTrack.front().first;
      vertTrack.push_back(std::make_pair(skin, m_nextOverPos++));
      return vertTrack.front().first;
    }

    template <class RigPair>
    atUint32 sendAdditionalVertsToBlender(hecl::blender::PyOutStream& os, const RigPair& rp, atUint32 baseVert) const {
      atUint32 addedVerts = 0;
      atUint32 nextVert = 1;
      while (nextVert < m_nextOverPos) {
        for (const std::pair<atUint16, std::vector<std::pair<atInt16, atUint16>>>& ev : m_extraVerts) {
          for (const std::pair<atInt16, atUint16>& se : ev.second) {
            if (se.second == nextVert) {
              os.format(fmt(
                  "bm.verts.ensure_lookup_table()\n"
                  "orig_vert = bm.verts[{}]\n"
                  "vert = bm.verts.new(orig_vert.co)\n"),
                  ev.first + baseVert);
              rp.first.second->weightVertex(os, *rp.second.second, se.first);
              ++nextVert;
              ++addedVerts;
            }
          }
        }
      }
      return addedVerts;
    }

    atUint16 lookupVertIdx(atUint16 pos, atInt16 skin) const {
      auto search = m_extraVerts.find(pos);
      if (search == m_extraVerts.end())
        return -1;
      const std::vector<std::pair<atInt16, atUint16>>& vertTrack = search->second;
      if (vertTrack.front().first == skin)
        return pos;
      for (auto it = vertTrack.begin() + 1; it != vertTrack.end(); ++it)
        if (it->first == skin)
          return m_maxBasePos + it->second;
      return -1;
    }
  };

private:
  const VertexAttributes& m_va;
  std::unique_ptr<atUint8[]> m_dl;
  size_t m_dlSize;
  ExtraVertTracker& m_evt;
  const atInt16* m_bankIn;
  atUint8* m_cur;
  atUint16 readVal(GX::AttrType type) {
    atUint16 retval = 0;
    switch (type) {
    case GX::DIRECT:
    case GX::INDEX8:
      if ((m_cur - m_dl.get()) >= intptr_t(m_dlSize))
        return 0;
      retval = *m_cur;
      ++m_cur;
      break;
    case GX::INDEX16:
      if ((m_cur - m_dl.get() + 1) >= intptr_t(m_dlSize))
        return 0;
      retval = hecl::SBig(*(atUint16*)m_cur);
      m_cur += 2;
      break;
    default:
      break;
    }
    return retval;
  }

public:
  DLReader(const VertexAttributes& va, std::unique_ptr<atUint8[]>&& dl, size_t dlSize, ExtraVertTracker& evt,
           const atInt16* bankIn = nullptr)
  : m_va(va), m_dl(std::move(dl)), m_dlSize(dlSize), m_evt(evt), m_bankIn(bankIn) {
    m_cur = m_dl.get();
  }

  explicit operator bool() const { return ((m_cur - m_dl.get()) < intptr_t(m_dlSize)) && *m_cur; }

  GX::Primitive readPrimitive() { return GX::Primitive(*m_cur++ & 0xf8); }

  GX::Primitive readPrimitiveAndVat(unsigned& vatOut) {
    atUint8 val = *m_cur++;
    vatOut = val & 0x7;
    return GX::Primitive(val & 0xf8);
  }

  atUint16 readVertCount() {
    atUint16 retval = hecl::SBig(*(atUint16*)m_cur);
    m_cur += 2;
    return retval;
  }

  struct DLPrimVert {
    atUint16 pos = 0;
    atUint16 norm = 0;
    atUint16 color[2] = {0};
    atUint16 uvs[7] = {0};
    atUint8 pnMtxIdx = 0;
    atUint8 texMtxIdx[7] = {0};
  };

  DLPrimVert readVert(bool peek = false) {
    atUint8* bakCur = m_cur;
    DLPrimVert retval;
    retval.pnMtxIdx = readVal(m_va.pnMtxIdx);
    retval.texMtxIdx[0] = readVal(m_va.texMtxIdx[0]);
    retval.texMtxIdx[1] = readVal(m_va.texMtxIdx[1]);
    retval.texMtxIdx[2] = readVal(m_va.texMtxIdx[2]);
    retval.texMtxIdx[3] = readVal(m_va.texMtxIdx[3]);
    retval.texMtxIdx[4] = readVal(m_va.texMtxIdx[4]);
    retval.texMtxIdx[5] = readVal(m_va.texMtxIdx[5]);
    retval.texMtxIdx[6] = readVal(m_va.texMtxIdx[6]);
    if (m_bankIn) {
      atUint16 posIdx = readVal(m_va.pos);
      atUint8 mtxIdx = retval.pnMtxIdx / 3;
      atInt16 skinIdx = -1;
      if (mtxIdx < 10)
        skinIdx = m_bankIn[mtxIdx];
      retval.pos = m_evt.lookupVertIdx(posIdx, skinIdx);
    } else
      retval.pos = readVal(m_va.pos);
    retval.norm = readVal(m_va.norm);
    retval.color[0] = readVal(m_va.color0);
    retval.color[1] = readVal(m_va.color1);
    retval.uvs[0] = readVal(m_va.uvs[0]);
    retval.uvs[1] = readVal(m_va.uvs[1]);
    retval.uvs[2] = readVal(m_va.uvs[2]);
    retval.uvs[3] = readVal(m_va.uvs[3]);
    retval.uvs[4] = readVal(m_va.uvs[4]);
    retval.uvs[5] = readVal(m_va.uvs[5]);
    retval.uvs[6] = readVal(m_va.uvs[6]);
    if (peek)
      m_cur = bakCur;
    return retval;
  }

  void preReadMaxIdxs(DLPrimVert& out) {
    atUint8* bakCur = m_cur;
    while (*this) {
      readPrimitive();
      atUint16 vc = readVertCount();
      for (atUint16 v = 0; v < vc; ++v) {
        atUint16 val;
        val = readVal(m_va.pnMtxIdx);
        out.pnMtxIdx = std::max(out.pnMtxIdx, atUint8(val));
        val = readVal(m_va.texMtxIdx[0]);
        out.texMtxIdx[0] = std::max(out.texMtxIdx[0], atUint8(val));
        val = readVal(m_va.texMtxIdx[1]);
        out.texMtxIdx[1] = std::max(out.texMtxIdx[1], atUint8(val));
        val = readVal(m_va.texMtxIdx[2]);
        out.texMtxIdx[2] = std::max(out.texMtxIdx[2], atUint8(val));
        val = readVal(m_va.texMtxIdx[3]);
        out.texMtxIdx[3] = std::max(out.texMtxIdx[3], atUint8(val));
        val = readVal(m_va.texMtxIdx[4]);
        out.texMtxIdx[4] = std::max(out.texMtxIdx[4], atUint8(val));
        val = readVal(m_va.texMtxIdx[5]);
        out.texMtxIdx[5] = std::max(out.texMtxIdx[5], atUint8(val));
        val = readVal(m_va.texMtxIdx[6]);
        out.texMtxIdx[6] = std::max(out.texMtxIdx[6], atUint8(val));
        val = readVal(m_va.pos);
        out.pos = std::max(out.pos, val);
        val = readVal(m_va.norm);
        out.norm = std::max(out.norm, val);
        val = readVal(m_va.color0);
        out.color[0] = std::max(out.color[0], val);
        val = readVal(m_va.color1);
        out.color[1] = std::max(out.color[1], val);
        val = readVal(m_va.uvs[0]);
        out.uvs[0] = std::max(out.uvs[0], val);
        val = readVal(m_va.uvs[1]);
        out.uvs[1] = std::max(out.uvs[1], val);
        val = readVal(m_va.uvs[2]);
        out.uvs[2] = std::max(out.uvs[2], val);
        val = readVal(m_va.uvs[3]);
        out.uvs[3] = std::max(out.uvs[3], val);
        val = readVal(m_va.uvs[4]);
        out.uvs[4] = std::max(out.uvs[4], val);
        val = readVal(m_va.uvs[5]);
        out.uvs[5] = std::max(out.uvs[5], val);
        val = readVal(m_va.uvs[6]);
        out.uvs[6] = std::max(out.uvs[6], val);
      }
    }
    m_cur = bakCur;
  }

  void preReadMaxIdxs(DLPrimVert& out, std::vector<atInt16>& skinOut) {
    atUint8* bakCur = m_cur;
    while (*this) {
      readPrimitive();
      atUint16 vc = readVertCount();
      for (atUint16 v = 0; v < vc; ++v) {
        atUint16 val;
        atUint8 pnMtxVal = readVal(m_va.pnMtxIdx);
        out.pnMtxIdx = std::max(out.pnMtxIdx, pnMtxVal);
        val = readVal(m_va.texMtxIdx[0]);
        out.texMtxIdx[0] = std::max(out.texMtxIdx[0], atUint8(val));
        val = readVal(m_va.texMtxIdx[1]);
        out.texMtxIdx[1] = std::max(out.texMtxIdx[1], atUint8(val));
        val = readVal(m_va.texMtxIdx[2]);
        out.texMtxIdx[2] = std::max(out.texMtxIdx[2], atUint8(val));
        val = readVal(m_va.texMtxIdx[3]);
        out.texMtxIdx[3] = std::max(out.texMtxIdx[3], atUint8(val));
        val = readVal(m_va.texMtxIdx[4]);
        out.texMtxIdx[4] = std::max(out.texMtxIdx[4], atUint8(val));
        val = readVal(m_va.texMtxIdx[5]);
        out.texMtxIdx[5] = std::max(out.texMtxIdx[5], atUint8(val));
        val = readVal(m_va.texMtxIdx[6]);
        out.texMtxIdx[6] = std::max(out.texMtxIdx[6], atUint8(val));
        atUint16 posVal = readVal(m_va.pos);
        out.pos = std::max(out.pos, posVal);
        val = readVal(m_va.norm);
        out.norm = std::max(out.norm, val);
        val = readVal(m_va.color0);
        out.color[0] = std::max(out.color[0], val);
        val = readVal(m_va.color1);
        out.color[1] = std::max(out.color[1], val);
        val = readVal(m_va.uvs[0]);
        out.uvs[0] = std::max(out.uvs[0], val);
        val = readVal(m_va.uvs[1]);
        out.uvs[1] = std::max(out.uvs[1], val);
        val = readVal(m_va.uvs[2]);
        out.uvs[2] = std::max(out.uvs[2], val);
        val = readVal(m_va.uvs[3]);
        out.uvs[3] = std::max(out.uvs[3], val);
        val = readVal(m_va.uvs[4]);
        out.uvs[4] = std::max(out.uvs[4], val);
        val = readVal(m_va.uvs[5]);
        out.uvs[5] = std::max(out.uvs[5], val);
        val = readVal(m_va.uvs[6]);
        out.uvs[6] = std::max(out.uvs[6], val);

        atInt16 skinIdx = m_bankIn[pnMtxVal / 3];
        skinOut[posVal] = m_evt.addPosSkinPair(posVal, skinIdx);
      }
    }
    m_cur = bakCur;
  }
};

void InitGeomBlenderContext(hecl::blender::PyOutStream& os, const hecl::ProjectPath& masterShaderPath) {
  os << "import math\n"
        "from mathutils import Vector\n"
        "\n"
        "# Clear Scene\n"
        "if len(bpy.data.collections):\n"
        "    bpy.data.collections.remove(bpy.data.collections[0])\n"
        "\n"
        "def loop_from_facevert(bm, face, vert_idx):\n"
        "    for loop in face.loops:\n"
        "        if loop.vert[bm.verts.layers.int['CMDLOriginalPosIdxs']] == vert_idx:\n"
        "            return loop\n"
        "\n"
        "def loops_from_edgevert(bm, edge, vert):\n"
        "    ret = []\n"
        "    for face in edge.link_faces:\n"
        "        for loop in face.loops:\n"
        "            if loop.vert == vert:\n"
        "                ret.append(loop)\n"
        "    return ret\n"
        "\n"
        "def add_triangle(bm, vert_seq, vert_indices, norm_seq, norm_indices, mat_nr, od_list, two_face_vert):\n"
        "    if len(set(vert_indices)) != 3:\n"
        "        return None, None\n"
        "\n"
        "    ret_mesh = bm\n"
        "    vert_seq.ensure_lookup_table()\n"
        "    verts = [vert_seq[i] for i in vert_indices]\n"
        "\n"
        "    # Try getting existing face\n"
        "    face = bm.faces.get(verts)\n"
        "\n"
        "    if face is not None and face.material_index != mat_nr: # Same poly, new material\n"
        "        # Overdraw detected; track copy\n"
        "        od_entry = None\n"
        "        for entry in od_list:\n"
        "            if entry['material'] == mat_nr:\n"
        "                od_entry = entry\n"
        "        if od_entry is None:\n"
        "            bm_cpy = bm.copy()\n"
        "            od_entry = {'material':mat_nr, 'bm':bm_cpy}\n"
        "            bmesh.ops.delete(od_entry['bm'], geom=od_entry['bm'].faces, context='FACES_ONLY')\n"
        "            od_list.append(od_entry)\n"
        "        od_entry['bm'].verts.ensure_lookup_table()\n"
        "        verts = [od_entry['bm'].verts[i] for i in vert_indices]\n"
        "        face = od_entry['bm'].faces.get(verts)\n"
        "        if face is None:\n"
        "            face = od_entry['bm'].faces.new(verts)\n"
        "        else: # Probably a double-sided surface\n"
        "            verts = [od_entry['bm'].verts[i + two_face_vert] for i in vert_indices]\n"
        "            face = od_entry['bm'].faces.get(verts)\n"
        "            if face is None:\n"
        "                face = od_entry['bm'].faces.new(verts)\n"
        "        ret_mesh = od_entry['bm']\n"
        "\n"
        "    elif face is not None: # Same material, probably double-sided\n"
        "        verts = [vert_seq[i + two_face_vert] for i in vert_indices]\n"
        "        face = bm.faces.get(verts)\n"
        "        if face is None:\n"
        "            face = bm.faces.new(verts)\n"
        "\n"
        "    else: # Make totally new face\n"
        "        face = bm.faces.new(verts)\n"
        "\n"
        "    for i in range(3):\n"
        "        face.verts[i][ret_mesh.verts.layers.int['CMDLOriginalPosIdxs']] = vert_indices[i]\n"
        "        face.loops[i][ret_mesh.loops.layers.int['CMDLOriginalNormIdxs']] = norm_indices[i]\n"
        "    face.material_index = mat_nr\n"
        "    face.smooth = True\n"
        "\n"
        "    return face, ret_mesh\n"
        "\n"
        "def expand_lightmap_triangle(lightmap_tri_tracker, uva, uvb, uvc):\n"
        "    result = ([uva[0],uva[1]], [uvb[0],uvb[1]], [uvc[0],uvc[1]])\n"
        "    inst = 0\n"
        "    if uva in lightmap_tri_tracker:\n"
        "        inst = lightmap_tri_tracker[uva]\n"
        "    lightmap_tri_tracker[uva] = inst + 1\n"
        "    if uva == uvb:\n"
        "        result[1][0] += 0.005\n"
        "    if uva == uvc:\n"
        "        result[2][1] -= 0.005\n"
        "    if inst & 0x1 and uva == uvb and uva == uvc:\n"
        "        result[0][0] += 0.005\n"
        "        result[0][1] -= 0.005\n"
        "    return result\n"
        "\n";

  /* Link master shader library */
  os.format(fmt(
      "# Master shader library\n"
      "with bpy.data.libraries.load('{}', link=True, relative=True) as (data_from, data_to):\n"
      "    data_to.node_groups = data_from.node_groups\n"
      "\n"),
      masterShaderPath.getAbsolutePathUTF8());
}

void FinishBlenderMesh(hecl::blender::PyOutStream& os, unsigned matSetCount, int meshIdx) {
  os << "if 'Render' not in bpy.data.collections:\n"
        "    coll = bpy.data.collections.new('Render')\n"
        "    bpy.context.scene.collection.children.link(coll)\n"
        "else:\n"
        "    coll = bpy.data.collections['Render']\n";
  if (meshIdx < 0) {
    os << "mesh = bpy.data.meshes.new(bpy.context.scene.name)\n"
          "obj = bpy.data.objects.new(mesh.name, mesh)\n"
          "obj.show_transparent = True\n"
          "coll.objects.link(obj)\n";
    os.format(fmt("mesh.hecl_material_count = {}\n"), matSetCount);
  } else {
    os.format(fmt("mesh = bpy.data.meshes.new(bpy.context.scene.name + '_{:03d}')\n"), meshIdx);
    os << "obj = bpy.data.objects.new(mesh.name, mesh)\n"
          "obj.show_transparent = True\n"
          "coll.objects.link(obj)\n";
    os.format(fmt("mesh.hecl_material_count = {}\n"), matSetCount);
  }

  os << "mesh.use_auto_smooth = True\n"
        "mesh.auto_smooth_angle = math.pi\n"
        "\n"
        "for material in materials:\n"
        "    mesh.materials.append(material)\n"
        "\n"
        "# Merge OD meshes\n"
        "for od_entry in od_list:\n"
        "    vert_dict = [{},{}]\n"
        "\n"
        "    for vert in od_entry['bm'].verts:\n"
        "        if len(vert.link_faces):\n"
        "            if vert.index >= two_face_vert:\n"
        "                use_vert_dict = vert_dict[1]\n"
        "            else:\n"
        "                use_vert_dict = vert_dict[0]\n"
        "            copy_vert = bm.verts.new(vert.co, vert)\n"
        "            use_vert_dict[vert[od_entry['bm'].verts.layers.int['CMDLOriginalPosIdxs']]] = copy_vert\n"
        "            copy_vert[orig_pidx_lay] = vert[od_entry['bm'].verts.layers.int['CMDLOriginalPosIdxs']]\n"
        "\n"
        "    for face in od_entry['bm'].faces:\n"
        "        if face.verts[0].index >= two_face_vert:\n"
        "            use_vert_dict = vert_dict[1]\n"
        "        else:\n"
        "            use_vert_dict = vert_dict[0]\n"
        "        merge_verts = [use_vert_dict[fv[od_entry['bm'].verts.layers.int['CMDLOriginalPosIdxs']]] for fv in "
        "face.verts]\n"
        "        if bm.faces.get(merge_verts) is not None:\n"
        "            continue\n"
        "        merge_face = bm.faces.new(merge_verts)\n"
        "        for i in range(len(face.loops)):\n"
        "            old = face.loops[i]\n"
        "            new = merge_face.loops[i]\n"
        "            for j in range(len(od_entry['bm'].loops.layers.uv)):\n"
        "                new[bm.loops.layers.uv[j]] = old[od_entry['bm'].loops.layers.uv[j]]\n"
        "            new[orig_nidx_lay] = old[od_entry['bm'].loops.layers.int['CMDLOriginalNormIdxs']]\n"
        "        merge_face.smooth = True\n"
        "        merge_face.material_index = face.material_index\n"
        "\n"
        "    od_entry['bm'].free()\n"
        "\n"
        "verts_to_del = []\n"
        "for v in bm.verts:\n"
        "    if len(v.link_faces) == 0:\n"
        "        verts_to_del.append(v)\n"
        "bmesh.ops.delete(bm, geom=verts_to_del, context='VERTS')\n"
        "\n"
        "for edge in bm.edges:\n"
        "    if edge.is_manifold:\n"
        "        pass_count = 0\n"
        "        for vert in edge.verts:\n"
        "            loops = loops_from_edgevert(bm, edge, vert)\n"
        "            norm0 = Vector(norm_list[loops[0][orig_nidx_lay]])\n"
        "            norm1 = Vector(norm_list[loops[1][orig_nidx_lay]])\n"
        "            if norm0.dot(norm1) < 0.9:\n"
        "                pass_count += 1\n"
        "        if pass_count > 0:\n"
        "            edge.smooth = False\n"
        "\n"
        "bm.to_mesh(mesh)\n"
        "bm.free()\n"
        "\n"
        "# Remove redundant materials\n"
        "present_mats = set()\n"
        "for poly in mesh.polygons:\n"
        "    present_mats.add(poly.material_index)\n"
        "for mat_idx in reversed(range(len(mesh.materials))):\n"
        "    if mat_idx not in present_mats:\n"
        "        mesh.materials.pop(index=mat_idx)\n"
        "\n"
        "mesh.update()\n"
        "\n";
}

template <class PAKRouter, class MaterialSet, class RigPair, class SurfaceHeader>
atUint32 ReadGeomSectionsToBlender(hecl::blender::PyOutStream& os, athena::io::IStreamReader& reader,
                                   PAKRouter& pakRouter, const typename PAKRouter::EntryType& entry, const RigPair& rp,
                                   bool shortNormals, bool shortUVs, std::vector<VertexAttributes>& vertAttribs,
                                   int meshIdx, atUint32 secCount, atUint32 matSetCount, const atUint32* secSizes,
                                   atUint32 surfaceCount) {
  os << "# Begin bmesh\n"
        "bm = bmesh.new()\n"
        "\n"
        "# Overdraw-tracking\n"
        "od_list = []\n"
        "\n"
        "orig_pidx_lay = bm.verts.layers.int.new('CMDLOriginalPosIdxs')\n"
        "orig_nidx_lay = bm.loops.layers.int.new('CMDLOriginalNormIdxs')\n"
        "\n"
        "lightmap_tri_tracker = {}\n";

  if (rp.first.second)
    os << "dvert_lay = bm.verts.layers.deform.verify()\n";

  /* Pre-read pass to determine maximum used vert indices */
  atUint32 matSecCount = 0;
  if (matSetCount)
    matSecCount = MaterialSet::OneSection() ? 1 : matSetCount;
  bool visitedDLOffsets = false;
  atUint32 lastDlSec = secCount;
  atUint64 afterHeaderPos = reader.position();
  DLReader::DLPrimVert maxIdxs;
  std::vector<atInt16> skinIndices;
  DLReader::ExtraVertTracker extraTracker;
  for (size_t s = 0; s < lastDlSec; ++s) {
    atUint64 secStart = reader.position();
    if (s < matSecCount) {
      if (!s) {
        MaterialSet matSet;
        matSet.read(reader);
        matSet.ensureTexturesExtracted(pakRouter);
        GetVertexAttributes(matSet, vertAttribs);
      }
    } else {
      switch (s - matSecCount) {
      case 0: {
        /* Positions */
        if (SurfaceHeader::UseMatrixSkinning() && rp.first.second)
          skinIndices.assign(secSizes[s] / 12, -1);
        break;
      }
      case 1: {
        /* Normals */
        break;
      }
      case 2: {
        /* Colors */
        break;
      }
      case 3: {
        /* Float UVs */
        break;
      }
      case 4: {
        if (surfaceCount) {
          /* MP3 MREA case */
          visitedDLOffsets = true;
          lastDlSec = 4 + surfaceCount;
        } else {
          /* Short UVs */
          if (shortUVs)
            break;

          /* DL Offsets (here or next section) */
          visitedDLOffsets = true;
          lastDlSec = s + reader.readUint32Big() + 1;
          break;
        }
        [[fallthrough]];
      }
      default: {
        if (!visitedDLOffsets) {
          visitedDLOffsets = true;
          lastDlSec = s + reader.readUint32Big() + 1;
          break;
        }

        /* GX Display List (surface) */
        SurfaceHeader sHead;
        sHead.read(reader);
        const atInt16* bankIn = nullptr;
        if (SurfaceHeader::UseMatrixSkinning() && rp.first.second)
          bankIn = rp.first.second->getMatrixBank(sHead.skinMatrixBankIdx());

        /* Do max index pre-read */
        atUint32 realDlSize = secSizes[s] - (reader.position() - secStart);
        DLReader dl(vertAttribs[sHead.matIdx], reader.readUBytes(realDlSize), realDlSize, extraTracker, bankIn);
        if (SurfaceHeader::UseMatrixSkinning() && rp.first.second)
          dl.preReadMaxIdxs(maxIdxs, skinIndices);
        else
          dl.preReadMaxIdxs(maxIdxs);
      }
      }
    }

    if (s < secCount - 1) {
      reader.seek(secStart + secSizes[s], athena::SeekOrigin::Begin);
    }
  }

  reader.seek(afterHeaderPos, athena::SeekOrigin::Begin);

  visitedDLOffsets = false;
  unsigned createdUVLayers = 0;
  unsigned surfIdx = 0;

  for (size_t s = 0; s < lastDlSec; ++s) {
    atUint64 secStart = reader.position();
    if (s < matSecCount) {
      MaterialSet matSet;
      matSet.read(reader);
      matSet.readToBlender(os, pakRouter, entry, s);
      if (!s)
        GetVertexAttributes(matSet, vertAttribs);
    } else {
      switch (s - matSecCount) {
      case 0: {
        /* Positions */
        atUint32 vertCount = maxIdxs.pos + 1;
        std::vector<atVec3f> positions;
        positions.reserve(vertCount);
        for (size_t i = 0; i <= maxIdxs.pos; ++i) {
          positions.push_back(reader.readVec3fBig());
          const atVec3f& pos = positions.back();
          os.format(fmt("vert = bm.verts.new(({},{},{}))\n"), pos.simd[0], pos.simd[1], pos.simd[2]);
          if (rp.first.second) {
            if (SurfaceHeader::UseMatrixSkinning() && !skinIndices.empty())
              rp.first.second->weightVertex(os, *rp.second.second, skinIndices[i]);
            else if (!SurfaceHeader::UseMatrixSkinning())
              rp.first.second->weightVertex(os, *rp.second.second, i);
          }
        }
        if (rp.first.second && SurfaceHeader::UseMatrixSkinning() && !skinIndices.empty())
          vertCount += extraTracker.sendAdditionalVertsToBlender(os, rp, 0);
        os.format(fmt("two_face_vert = {}\n"), vertCount);
        for (size_t i = 0; i <= maxIdxs.pos; ++i) {
          const atVec3f& pos = positions[i];
          os.format(fmt("vert = bm.verts.new(({},{},{}))\n"), pos.simd[0], pos.simd[1], pos.simd[2]);
          if (rp.first.second) {
            if (SurfaceHeader::UseMatrixSkinning() && !skinIndices.empty())
              rp.first.second->weightVertex(os, *rp.second.second, skinIndices[i]);
            else if (!SurfaceHeader::UseMatrixSkinning())
              rp.first.second->weightVertex(os, *rp.second.second, i);
          }
        }
        if (rp.first.second && SurfaceHeader::UseMatrixSkinning() && !skinIndices.empty())
          extraTracker.sendAdditionalVertsToBlender(os, rp, vertCount);
        break;
      }
      case 1: {
        /* Normals */
        os << "norm_list = []\n";
        if (shortNormals) {
          size_t normCount = secSizes[s] / 6;
          for (size_t i = 0; i < normCount; ++i) {
            float x = reader.readInt16Big() / 16384.0f;
            float y = reader.readInt16Big() / 16384.0f;
            float z = reader.readInt16Big() / 16384.0f;
            os.format(fmt("norm_list.append(({},{},{}))\n"), x, y, z);
          }
        } else {
          size_t normCount = secSizes[s] / 12;
          for (size_t i = 0; i < normCount; ++i) {
            const atVec3f norm = reader.readVec3fBig();
            os.format(fmt("norm_list.append(({},{},{}))\n"), norm.simd[0], norm.simd[1], norm.simd[2]);
          }
        }
        break;
      }
      case 2: {
        /* Colors */
        break;
      }
      case 3: {
        /* Float UVs */
        os << "uv_list = []\n";
        size_t uvCount = secSizes[s] / 8;
        for (size_t i = 0; i < uvCount; ++i) {
          const atVec2f uv = reader.readVec2fBig();
          os.format(fmt("uv_list.append(({},{}))\n"), uv.simd[0], uv.simd[1]);
        }
        break;
      }
      case 4: {
        if (surfaceCount) {
          /* MP3 MREA case */
          visitedDLOffsets = true;
        } else {
          /* Short UVs */
          os << "suv_list = []\n";
          if (shortUVs) {
            size_t uvCount = secSizes[s] / 4;
            for (size_t i = 0; i < uvCount; ++i) {
              float x = reader.readInt16Big() / 32768.0f;
              float y = reader.readInt16Big() / 32768.0f;
              os.format(fmt("suv_list.append(({},{}))\n"), x, y);
            }
            break;
          }

          /* DL Offsets (here or next section) */
          visitedDLOffsets = true;
          break;
        }
        [[fallthrough]];
      }
      default: {
        if (!visitedDLOffsets) {
          visitedDLOffsets = true;
          break;
        }

        /* GX Display List (surface) */
        SurfaceHeader sHead;
        sHead.read(reader);
        VertexAttributes& curVA = vertAttribs[sHead.matIdx];
        unsigned matUVCount = curVA.uvCount;
        bool matShortUVs = curVA.shortUVs;
        const atInt16* bankIn = nullptr;
        if (SurfaceHeader::UseMatrixSkinning() && rp.first.second)
          bankIn = rp.first.second->getMatrixBank(sHead.skinMatrixBankIdx());

        os.format(fmt("materials[{}].pass_index = {}\n"), sHead.matIdx, surfIdx++);
        if (matUVCount > createdUVLayers) {
          for (unsigned l = createdUVLayers; l < matUVCount; ++l)
            os.format(fmt("bm.loops.layers.uv.new('UV_{}')\n"), l);
          createdUVLayers = matUVCount;
        }

        atUint32 realDlSize = secSizes[s] - (reader.position() - secStart);
        DLReader dl(vertAttribs[sHead.matIdx], reader.readUBytes(realDlSize), realDlSize, extraTracker, bankIn);

        while (dl) {
          GX::Primitive ptype = dl.readPrimitive();
          atUint16 vertCount = dl.readVertCount();

          /* First vert */
          DLReader::DLPrimVert firstPrimVert = dl.readVert(true);

          /* 3 Prim Verts to start */
          int c = 0;
          DLReader::DLPrimVert primVerts[3] = {dl.readVert(), dl.readVert(), dl.readVert()};

          if (ptype == GX::TRIANGLESTRIP) {
            atUint8 flip = 0;
            for (int v = 0; v < vertCount - 2; ++v) {
              if (flip) {
                os.format(fmt(
                    "last_face, last_mesh = add_triangle(bm, bm.verts, ({},{},{}), norm_list, ({},{},{}), {}, od_list, "
                    "two_face_vert)\n"),
                    primVerts[c % 3].pos, primVerts[(c + 2) % 3].pos, primVerts[(c + 1) % 3].pos, primVerts[c % 3].norm,
                    primVerts[(c + 2) % 3].norm, primVerts[(c + 1) % 3].norm, sHead.matIdx);
                if (matUVCount) {
                  os << "if last_face is not None:\n";
                  for (unsigned j = 0; j < matUVCount; ++j) {
                    if (j == 0 && matShortUVs)
                      os.format(fmt(
                          "    uv_tri = expand_lightmap_triangle(lightmap_tri_tracker, suv_list[{}], suv_list[{}], "
                          "suv_list[{}])\n"
                          "    loop_from_facevert(last_mesh, last_face, {})[last_mesh.loops.layers.uv[{}]].uv = "
                          "uv_tri[0]\n"
                          "    loop_from_facevert(last_mesh, last_face, {})[last_mesh.loops.layers.uv[{}]].uv = "
                          "uv_tri[1]\n"
                          "    loop_from_facevert(last_mesh, last_face, {})[last_mesh.loops.layers.uv[{}]].uv = "
                          "uv_tri[2]\n"),
                          primVerts[c % 3].uvs[j], primVerts[(c + 2) % 3].uvs[j], primVerts[(c + 1) % 3].uvs[j],
                          primVerts[c % 3].pos, j, primVerts[(c + 2) % 3].pos, j, primVerts[(c + 1) % 3].pos, j);
                    else
                      os.format(fmt(
                          "    loop_from_facevert(last_mesh, last_face, {})[last_mesh.loops.layers.uv[{}]].uv = "
                          "uv_list[{}]\n"
                          "    loop_from_facevert(last_mesh, last_face, {})[last_mesh.loops.layers.uv[{}]].uv = "
                          "uv_list[{}]\n"
                          "    loop_from_facevert(last_mesh, last_face, {})[last_mesh.loops.layers.uv[{}]].uv = "
                          "uv_list[{}]\n"),
                          primVerts[c % 3].pos, j, primVerts[c % 3].uvs[j], primVerts[(c + 2) % 3].pos, j,
                          primVerts[(c + 2) % 3].uvs[j], primVerts[(c + 1) % 3].pos, j, primVerts[(c + 1) % 3].uvs[j]);
                  }
                }
              } else {
                os.format(fmt(
                    "last_face, last_mesh = add_triangle(bm, bm.verts, ({},{},{}), norm_list, ({},{},{}), {}, od_list, "
                    "two_face_vert)\n"),
                    primVerts[c % 3].pos, primVerts[(c + 1) % 3].pos, primVerts[(c + 2) % 3].pos, primVerts[c % 3].norm,
                    primVerts[(c + 1) % 3].norm, primVerts[(c + 2) % 3].norm, sHead.matIdx);
                if (matUVCount) {
                  os << "if last_face is not None:\n";
                  for (unsigned j = 0; j < matUVCount; ++j) {
                    if (j == 0 && matShortUVs)
                      os.format(fmt(
                          "    uv_tri = expand_lightmap_triangle(lightmap_tri_tracker, suv_list[{}], suv_list[{}], "
                          "suv_list[{}])\n"
                          "    loop_from_facevert(last_mesh, last_face, {})[last_mesh.loops.layers.uv[{}]].uv = "
                          "uv_tri[0]\n"
                          "    loop_from_facevert(last_mesh, last_face, {})[last_mesh.loops.layers.uv[{}]].uv = "
                          "uv_tri[1]\n"
                          "    loop_from_facevert(last_mesh, last_face, {})[last_mesh.loops.layers.uv[{}]].uv = "
                          "uv_tri[2]\n"),
                          primVerts[c % 3].uvs[j], primVerts[(c + 1) % 3].uvs[j], primVerts[(c + 2) % 3].uvs[j],
                          primVerts[c % 3].pos, j, primVerts[(c + 1) % 3].pos, j, primVerts[(c + 2) % 3].pos, j);
                    else
                      os.format(fmt(
                          "    loop_from_facevert(last_mesh, last_face, {})[last_mesh.loops.layers.uv[{}]].uv = "
                          "uv_list[{}]\n"
                          "    loop_from_facevert(last_mesh, last_face, {})[last_mesh.loops.layers.uv[{}]].uv = "
                          "uv_list[{}]\n"
                          "    loop_from_facevert(last_mesh, last_face, {})[last_mesh.loops.layers.uv[{}]].uv = "
                          "uv_list[{}]\n"),
                          primVerts[c % 3].pos, j, primVerts[c % 3].uvs[j], primVerts[(c + 1) % 3].pos, j,
                          primVerts[(c + 1) % 3].uvs[j], primVerts[(c + 2) % 3].pos, j, primVerts[(c + 2) % 3].uvs[j]);
                  }
                }
              }
              flip ^= 1;

              bool peek = (v >= vertCount - 3);

              /* Advance one prim vert */
              primVerts[c % 3] = dl.readVert(peek);
              ++c;
            }
          } else if (ptype == GX::TRIANGLES) {
            for (int v = 0; v < vertCount; v += 3) {

              os.format(fmt(
                  "last_face, last_mesh = add_triangle(bm, bm.verts, ({},{},{}), norm_list, ({},{},{}), {}, od_list, "
                  "two_face_vert)\n"),
                  primVerts[0].pos, primVerts[1].pos, primVerts[2].pos, primVerts[0].norm, primVerts[1].norm,
                  primVerts[2].norm, sHead.matIdx);
              if (matUVCount) {
                os << "if last_face is not None:\n";
                for (unsigned j = 0; j < matUVCount; ++j) {
                  if (j == 0 && matShortUVs)
                    os.format(fmt(
                        "    uv_tri = expand_lightmap_triangle(lightmap_tri_tracker, suv_list[{}], suv_list[{}], "
                        "suv_list[{}])\n"
                        "    loop_from_facevert(last_mesh, last_face, {})[last_mesh.loops.layers.uv[{}]].uv = "
                        "uv_tri[0]\n"
                        "    loop_from_facevert(last_mesh, last_face, {})[last_mesh.loops.layers.uv[{}]].uv = "
                        "uv_tri[1]\n"
                        "    loop_from_facevert(last_mesh, last_face, {})[last_mesh.loops.layers.uv[{}]].uv = "
                        "uv_tri[2]\n"),
                        primVerts[0].uvs[j], primVerts[1].uvs[j], primVerts[2].uvs[j], primVerts[0].pos, j,
                        primVerts[1].pos, j, primVerts[2].pos, j);
                  else
                    os.format(fmt(
                        "    loop_from_facevert(last_mesh, last_face, {})[last_mesh.loops.layers.uv[{}]].uv = "
                        "uv_list[{}]\n"
                        "    loop_from_facevert(last_mesh, last_face, {})[last_mesh.loops.layers.uv[{}]].uv = "
                        "uv_list[{}]\n"
                        "    loop_from_facevert(last_mesh, last_face, {})[last_mesh.loops.layers.uv[{}]].uv = "
                        "uv_list[{}]\n"),
                        primVerts[0].pos, j, primVerts[0].uvs[j], primVerts[1].pos, j, primVerts[1].uvs[j],
                        primVerts[2].pos, j, primVerts[2].uvs[j]);
                }
              }

              /* Break if done */
              if (v + 3 >= vertCount)
                break;

              /* Advance 3 Prim Verts */
              for (int pv = 0; pv < 3; ++pv)
                primVerts[pv] = dl.readVert();
            }
          } else if (ptype == GX::TRIANGLEFAN) {
            ++c;
            for (int v = 0; v < vertCount - 2; ++v) {
              os.format(fmt(
                  "last_face, last_mesh = add_triangle(bm, bm.verts, ({},{},{}), norm_list, ({},{},{}), {}, od_list, "
                  "two_face_vert)\n"),
                  firstPrimVert.pos, primVerts[c % 3].pos, primVerts[(c + 1) % 3].pos, firstPrimVert.norm,
                  primVerts[c % 3].norm, primVerts[(c + 1) % 3].norm, sHead.matIdx);
              if (matUVCount) {
                os << "if last_face is not None:\n";
                for (unsigned j = 0; j < matUVCount; ++j) {
                  if (j == 0 && matShortUVs)
                    os.format(fmt(
                        "    uv_tri = expand_lightmap_triangle(lightmap_tri_tracker, suv_list[{}], suv_list[{}], "
                        "suv_list[{}])\n"
                        "    loop_from_facevert(last_mesh, last_face, {})[last_mesh.loops.layers.uv[{}]].uv = "
                        "uv_tri[0]\n"
                        "    loop_from_facevert(last_mesh, last_face, {})[last_mesh.loops.layers.uv[{}]].uv = "
                        "uv_tri[1]\n"
                        "    loop_from_facevert(last_mesh, last_face, {})[last_mesh.loops.layers.uv[{}]].uv = "
                        "uv_tri[2]\n"),
                        firstPrimVert.uvs[j], primVerts[c % 3].uvs[j], primVerts[(c + 1) % 3].uvs[j], firstPrimVert.pos,
                        j, primVerts[c % 3].pos, j, primVerts[(c + 1) % 3].pos, j);
                  else
                    os.format(fmt(
                        "    loop_from_facevert(last_mesh, last_face, {})[last_mesh.loops.layers.uv[{}]].uv = "
                        "uv_list[{}]\n"
                        "    loop_from_facevert(last_mesh, last_face, {})[last_mesh.loops.layers.uv[{}]].uv = "
                        "uv_list[{}]\n"
                        "    loop_from_facevert(last_mesh, last_face, {})[last_mesh.loops.layers.uv[{}]].uv = "
                        "uv_list[{}]\n"),
                        firstPrimVert.pos, j, firstPrimVert.uvs[j], primVerts[c % 3].pos, j, primVerts[c % 3].uvs[j],
                        primVerts[(c + 1) % 3].pos, j, primVerts[(c + 1) % 3].uvs[j]);
                }
              }

              /* Break if done */
              if (v + 3 >= vertCount)
                break;

              /* Advance one prim vert */
              primVerts[(c + 2) % 3] = dl.readVert();
              ++c;
            }
          }
          os << "\n";
        }
      }
      }
    }

    if (s < secCount - 1) {
      reader.seek(secStart + secSizes[s], athena::SeekOrigin::Begin);
    }
  }

  /* Finish Mesh */
  FinishBlenderMesh(os, matSetCount, meshIdx);

  if (rp.first.second) {
    os.format(fmt("mesh.cskr_id = '{}'\n"), rp.first.first);
    rp.second.second->sendVertexGroupsToBlender(os);
  }

  return lastDlSec;
}

template <class PAKRouter, class MaterialSet, class RigPair, class SurfaceHeader, atUint32 Version>
bool ReadCMDLToBlender(hecl::blender::Connection& conn, athena::io::IStreamReader& reader, PAKRouter& pakRouter,
                       const typename PAKRouter::EntryType& entry, const SpecBase& dataspec, const RigPair& rp) {
  Header head;
  head.read(reader);

  if (head.magic != 0xDEADBABE) {
    LogDNACommon.report(logvisor::Error, fmt("invalid CMDL magic"));
    return false;
  }

  if (head.version != Version) {
    LogDNACommon.report(logvisor::Error, fmt("invalid CMDL version"));
    return false;
  }

  /* Open Py Stream and read sections */
  hecl::blender::PyOutStream os = conn.beginPythonOut(true);
  os.format(fmt(
      "import bpy\n"
      "import bmesh\n"
      "\n"
      "bpy.context.scene.name = '{}'\n"
      "bpy.context.scene.hecl_mesh_obj = bpy.context.scene.name\n"),
      pakRouter.getBestEntryName(entry));
  InitGeomBlenderContext(os, dataspec.getMasterShaderPath());
  MaterialSet::RegisterMaterialProps(os);

  os << "# Materials\n"
        "materials = []\n"
        "\n";

  std::vector<VertexAttributes> vertAttribs;
  ReadGeomSectionsToBlender<PAKRouter, MaterialSet, RigPair, SurfaceHeader>(
      os, reader, pakRouter, entry, rp, head.flags.shortNormals(), head.flags.shortUVs(), vertAttribs, -1,
      head.secCount, head.matSetCount, head.secSizes.data());

  return true;
}

template bool ReadCMDLToBlender<PAKRouter<DNAMP1::PAKBridge>, DNAMP1::MaterialSet,
    std::pair<std::pair<UniqueID32, DNAMP1::CSKR*>, std::pair<UniqueID32, DNAMP1::CINF*>>, DNACMDL::SurfaceHeader_1, 2>(
    hecl::blender::Connection& conn, athena::io::IStreamReader& reader, PAKRouter<DNAMP1::PAKBridge>& pakRouter,
    const PAKRouter<DNAMP1::PAKBridge>::EntryType& entry, const SpecBase& dataspec,
    const std::pair<std::pair<UniqueID32, DNAMP1::CSKR*>, std::pair<UniqueID32, DNAMP1::CINF*>>& rp);

template bool ReadCMDLToBlender<PAKRouter<DNAMP2::PAKBridge>, DNAMP2::MaterialSet,
    std::pair<std::pair<UniqueID32, DNAMP2::CSKR*>, std::pair<UniqueID32, DNAMP2::CINF*>>, DNACMDL::SurfaceHeader_2, 4>(
    hecl::blender::Connection& conn, athena::io::IStreamReader& reader, PAKRouter<DNAMP2::PAKBridge>& pakRouter,
    const PAKRouter<DNAMP2::PAKBridge>::EntryType& entry, const SpecBase& dataspec,
    const std::pair<std::pair<UniqueID32, DNAMP2::CSKR*>, std::pair<UniqueID32, DNAMP2::CINF*>>& rp);

template bool ReadCMDLToBlender<PAKRouter<DNAMP3::PAKBridge>, DNAMP3::MaterialSet,
    std::pair<std::pair<UniqueID64, DNAMP3::CSKR*>, std::pair<UniqueID64, DNAMP3::CINF*>>, DNACMDL::SurfaceHeader_3, 4>(
    hecl::blender::Connection& conn, athena::io::IStreamReader& reader, PAKRouter<DNAMP3::PAKBridge>& pakRouter,
    const PAKRouter<DNAMP3::PAKBridge>::EntryType& entry, const SpecBase& dataspec,
    const std::pair<std::pair<UniqueID64, DNAMP3::CSKR*>, std::pair<UniqueID64, DNAMP3::CINF*>>& rp);

template bool ReadCMDLToBlender<PAKRouter<DNAMP3::PAKBridge>, DNAMP3::MaterialSet,
    std::pair<std::pair<UniqueID64, DNAMP3::CSKR*>, std::pair<UniqueID64, DNAMP3::CINF*>>, DNACMDL::SurfaceHeader_3, 5>(
    hecl::blender::Connection& conn, athena::io::IStreamReader& reader, PAKRouter<DNAMP3::PAKBridge>& pakRouter,
    const PAKRouter<DNAMP3::PAKBridge>::EntryType& entry, const SpecBase& dataspec,
    const std::pair<std::pair<UniqueID64, DNAMP3::CSKR*>, std::pair<UniqueID64, DNAMP3::CINF*>>& rp);

template <class PAKRouter, class MaterialSet>
void NameCMDL(athena::io::IStreamReader& reader, PAKRouter& pakRouter, typename PAKRouter::EntryType& entry,
              const SpecBase& dataspec) {
  Header head;
  head.read(reader);
  std::string bestName = fmt::format(fmt("CMDL_{}"), entry.id);

  /* Pre-read pass to determine maximum used vert indices */
  atUint32 matSecCount = 0;
  if (head.matSetCount)
    matSecCount = MaterialSet::OneSection() ? 1 : head.matSetCount;
  atUint32 lastDlSec = head.secCount;
  for (size_t s = 0; s < lastDlSec; ++s) {
    atUint64 secStart = reader.position();
    if (s < matSecCount) {
      MaterialSet matSet;
      matSet.read(reader);
      matSet.nameTextures(pakRouter, bestName.c_str(), s);
    }

    if (s < head.secCount - 1) {
      reader.seek(secStart + head.secSizes[s], athena::SeekOrigin::Begin);
    }
  }
}

template void NameCMDL<PAKRouter<DNAMP1::PAKBridge>, DNAMP1::MaterialSet>(
    athena::io::IStreamReader& reader, PAKRouter<DNAMP1::PAKBridge>& pakRouter,
    PAKRouter<DNAMP1::PAKBridge>::EntryType& entry, const SpecBase& dataspec);

template <typename W>
static void WriteDLVal(W& writer, GX::AttrType type, atUint32 val) {
  switch (type) {
  case GX::DIRECT:
  case GX::INDEX8:
    writer.writeUByte(atUint8(val));
    break;
  case GX::INDEX16:
    writer.writeUint16Big(atUint16(val));
    break;
  default:
    break;
  }
}

template <class MaterialSet, class SurfaceHeader, atUint32 Version>
bool WriteCMDL(const hecl::ProjectPath& outPath, const hecl::ProjectPath& inPath, const Mesh& mesh) {
  bool skinned = !mesh.skins.empty();

  Header head;
  head.magic = 0xDEADBABE;
  head.version = Version;
  head.flags.setSkinned(skinned);
  head.flags.setShortNormals(!skinned);
  head.flags.setShortUVs(true); /* This just means there's an (empty) short UV section */
  head.aabbMin = mesh.aabbMin.val;
  head.aabbMax = mesh.aabbMax.val;
  head.matSetCount = mesh.materialSets.size();
  head.secCount = head.matSetCount + 6 + mesh.surfaces.size();
  head.secSizes.reserve(head.secCount);

  /* Lengths of padding to insert while writing */
  std::vector<size_t> paddingSizes;
  paddingSizes.reserve(head.secCount);

  /* Build material sets */
  std::vector<MaterialSet> matSets;
#if 0
  matSets.reserve(mesh.materialSets.size());
  {
    for (const std::vector<Material>& mset : mesh.materialSets) {
      matSets.emplace_back();
      MaterialSet& targetMSet = matSets.back();
      std::vector<hecl::ProjectPath> texPaths;
      std::vector<hecl::Backend::GX> setBackends;
      setBackends.reserve(mset.size());

      size_t endOff = 0;
      for (const Material& mat : mset) {
        std::string diagName = fmt::format(fmt("{}:{}"), inPath.getLastComponentUTF8(), mat.name);
        hecl::Frontend::IR matIR = FE.compileSource(mat.source, diagName);
        setBackends.emplace_back();
        hecl::Backend::GX& matGX = setBackends.back();
        matGX.reset(matIR, FE.getDiagnostics());

        targetMSet.materials.emplace_back(matGX, mat.iprops, mat.texs, texPaths, mesh.colorLayerCount, false, false);

        targetMSet.materials.back().binarySize(endOff);
        targetMSet.head.addMaterialEndOff(endOff);
      }

      texPaths.reserve(mset.size() * 4);
      for (const Material& mat : mset) {
        for (const hecl::ProjectPath& path : mat.texs) {
          bool found = false;
          for (const hecl::ProjectPath& ePath : texPaths) {
            if (path == ePath) {
              found = true;
              break;
            }
          }
          if (!found)
            texPaths.push_back(path);
        }
      }

      for (const hecl::ProjectPath& path : texPaths)
        targetMSet.head.addTexture(path);

      size_t secSz = 0;
      targetMSet.binarySize(secSz);
      size_t secSz32 = ROUND_UP_32(secSz);
      head.secSizes.push_back(secSz32);
      paddingSizes.push_back(secSz32 - secSz);
    }
  }
#endif

  /* Vertex Positions */
  size_t secSz = mesh.pos.size() * 12;
  size_t secSz32 = ROUND_UP_32(secSz);
  if (secSz32 == 0)
    secSz32 = 32;
  head.secSizes.push_back(secSz32);
  paddingSizes.push_back(secSz32 - secSz);

  /* Vertex Normals */
  secSz = mesh.norm.size() * (skinned ? 12 : 6);
  secSz32 = ROUND_UP_32(secSz);
  if (secSz32 == 0)
    secSz32 = 32;
  head.secSizes.push_back(secSz32);
  paddingSizes.push_back(secSz32 - secSz);

  /* Vertex Colors */
  secSz = mesh.color.size() * 4;
  secSz32 = ROUND_UP_32(secSz);
  if (secSz32 == 0)
    secSz32 = 32;
  head.secSizes.push_back(secSz32);
  paddingSizes.push_back(secSz32 - secSz);

  /* UV coords */
  secSz = mesh.uv.size() * 8;
  secSz32 = ROUND_UP_32(secSz);
  if (secSz32 == 0)
    secSz32 = 32;
  head.secSizes.push_back(secSz32);
  paddingSizes.push_back(secSz32 - secSz);

  /* LUV coords */
  secSz = 0;
  secSz32 = ROUND_UP_32(secSz);
  head.secSizes.push_back(secSz32);
  paddingSizes.push_back(secSz32 - secSz);

  /* Surface index */
  std::vector<size_t> surfEndOffs;
  surfEndOffs.reserve(mesh.surfaces.size());
  secSz = mesh.surfaces.size() * 4 + 4;
  secSz32 = ROUND_UP_32(secSz);
  if (secSz32 == 0)
    secSz32 = 32;
  head.secSizes.push_back(secSz32);
  paddingSizes.push_back(secSz32 - secSz);

  /* Surfaces */
  size_t endOff = 0;
  size_t firstSurfSec = head.secSizes.size();
  for (const Mesh::Surface& surf : mesh.surfaces) {
    size_t vertSz = matSets.at(0).materials.at(surf.materialIdx).getVAFlags().vertDLSize();
    if (surf.verts.size() > 65536)
      LogDNACommon.report(logvisor::Fatal, fmt("GX DisplayList overflow"));
    size_t secSz = 64;
    for (auto it = surf.verts.cbegin(); it != surf.verts.cend();) {
      atUint16 vertCount = 0;
      auto itEnd = surf.verts.cend();
      for (auto it2 = it; it2 != surf.verts.cend(); ++it2, ++vertCount)
        if (it2->iPos == 0xffffffff) {
          if (vertCount == 3) {
            /* All primitives here on out are triangles */
            vertCount = atUint16((surf.verts.cend() - it + 1) * 3 / 4);
            break;
          }
          itEnd = it2;
          break;
        }
      secSz += 3 + vertCount * vertSz;
      if (itEnd == surf.verts.cend())
        break;
      it = itEnd + 1;
    }
    secSz32 = ROUND_UP_32(secSz);
    if (secSz32 == 0)
      secSz32 = 32;
    head.secSizes.push_back(secSz32);
    paddingSizes.push_back(secSz32 - secSz);
    endOff += secSz32;
    surfEndOffs.push_back(endOff);
  }

  /* Write sections */
  athena::io::FileWriter writer(outPath.getAbsolutePath());
  head.write(writer);
  std::vector<size_t>::const_iterator padIt = paddingSizes.cbegin();

  /* Material Sets */
  for (const MaterialSet& mset : matSets) {
    mset.write(writer);
    writer.fill(atUint8(0), *padIt);
    ++padIt;
  }

  /* Vertex Positions */
  for (const atVec3f& pos : mesh.pos)
    writer.writeVec3fBig(pos);
  writer.fill(atUint8(0), *padIt);
  ++padIt;

  /* Vertex Normals */
  for (const atVec3f& norm : mesh.norm) {
    if (skinned) {
      writer.writeVec3fBig(norm);
    } else {
      for (int i = 0; i < 3; ++i) {
        int tmpV = int(norm.simd[i] * 16384.f);
        tmpV = zeus::clamp(-32768, tmpV, 32767);
        writer.writeInt16Big(atInt16(tmpV));
      }
    }
  }
  writer.fill(atUint8(0), *padIt);
  ++padIt;

  /* Vertex Colors */
  for (const atVec3f& col : mesh.color) {
    GX::Color qCol(col);
    qCol.write(writer);
  }
  writer.fill(atUint8(0), *padIt);
  ++padIt;

  /* UV coords */
  for (const atVec2f& uv : mesh.uv)
    writer.writeVec2fBig(uv);
  writer.fill(atUint8(0), *padIt);
  ++padIt;

  /* LUV coords */
  writer.fill(atUint8(0), *padIt);
  ++padIt;

  /* Surface index */
  writer.writeUint32Big(surfEndOffs.size());
  for (size_t off : surfEndOffs)
    writer.writeUint32Big(off);
  writer.fill(atUint8(0), *padIt);
  ++padIt;

  /* Surfaces */
  GX::Primitive prim = GX::TRIANGLES;
  if (mesh.topology == hecl::HMDLTopology::Triangles)
    prim = GX::TRIANGLES;
  else if (mesh.topology == hecl::HMDLTopology::TriStrips)
    prim = GX::TRIANGLESTRIP;
  else
    LogDNACommon.report(logvisor::Fatal, fmt("unrecognized mesh output mode"));
  auto surfSizeIt = head.secSizes.begin() + firstSurfSec;
  for (const Mesh::Surface& surf : mesh.surfaces) {
    const typename MaterialSet::Material::VAFlags& vaFlags = matSets.at(0).materials.at(surf.materialIdx).getVAFlags();

    SurfaceHeader header;
    header.centroid = surf.centroid;
    header.matIdx = surf.materialIdx;
    header.dlSize = (*surfSizeIt++ - 64) | 0x80000000;
    header.reflectionNormal = surf.reflectionNormal;
    header.write(writer);

    GX::Primitive usePrim = prim;
    for (auto it = surf.verts.cbegin(); it != surf.verts.cend();) {
      atUint16 vertCount = 0;
      auto itEnd = surf.verts.cend();
      for (auto it2 = it; it2 != surf.verts.cend(); ++it2, ++vertCount)
        if (it2->iPos == 0xffffffff) {
          if (vertCount == 3) {
            /* All primitives here on out are triangles */
            usePrim = GX::TRIANGLES;
            vertCount = atUint16((surf.verts.cend() - it + 1) * 3 / 4);
            break;
          }
          itEnd = it2;
          break;
        }

      /* VAT0 = float normals, float UVs
       * VAT1 = short normals, float UVs */
      writer.writeUByte(usePrim | (skinned ? 0x0 : 0x1));
      writer.writeUint16Big(vertCount);

      for (auto it2 = it; it2 != itEnd; ++it2) {
        const Mesh::Surface::Vert& vert = *it2;
        if (vert.iPos == 0xffffffff)
          continue;
        atUint32 skinIdx = vert.iBankSkin * 3;
        WriteDLVal(writer, vaFlags.pnMatIdx(), skinIdx);
        WriteDLVal(writer, vaFlags.tex0MatIdx(), skinIdx);
        WriteDLVal(writer, vaFlags.tex1MatIdx(), skinIdx);
        WriteDLVal(writer, vaFlags.tex2MatIdx(), skinIdx);
        WriteDLVal(writer, vaFlags.tex3MatIdx(), skinIdx);
        WriteDLVal(writer, vaFlags.tex4MatIdx(), skinIdx);
        WriteDLVal(writer, vaFlags.tex5MatIdx(), skinIdx);
        WriteDLVal(writer, vaFlags.tex6MatIdx(), skinIdx);
        WriteDLVal(writer, vaFlags.position(), vert.iPos);
        WriteDLVal(writer, vaFlags.normal(), vert.iNorm);
        WriteDLVal(writer, vaFlags.color0(), vert.iColor[0]);
        WriteDLVal(writer, vaFlags.color1(), vert.iColor[1]);
        WriteDLVal(writer, vaFlags.tex0(), vert.iUv[0]);
        WriteDLVal(writer, vaFlags.tex1(), vert.iUv[1]);
        WriteDLVal(writer, vaFlags.tex2(), vert.iUv[2]);
        WriteDLVal(writer, vaFlags.tex3(), vert.iUv[3]);
        WriteDLVal(writer, vaFlags.tex4(), vert.iUv[4]);
        WriteDLVal(writer, vaFlags.tex5(), vert.iUv[5]);
        WriteDLVal(writer, vaFlags.tex6(), vert.iUv[6]);
      }

      if (itEnd == surf.verts.cend())
        break;
      it = itEnd + 1;
    }

    writer.fill(atUint8(0), *padIt);
    ++padIt;
  }

  writer.close();
  return true;
}

template bool WriteCMDL<DNAMP1::MaterialSet, DNACMDL::SurfaceHeader_1, 2>(const hecl::ProjectPath& outPath,
                                                                          const hecl::ProjectPath& inPath,
                                                                          const Mesh& mesh);

template <class MaterialSet, class SurfaceHeader, atUint32 Version>
bool WriteHMDLCMDL(const hecl::ProjectPath& outPath, const hecl::ProjectPath& inPath, const Mesh& mesh,
                   hecl::blender::PoolSkinIndex& poolSkinIndex) {
  Header head;
  head.magic = 0xDEADBABE;
  head.version = 0x10000 | Version;
  head.aabbMin = mesh.aabbMin.val;
  head.aabbMax = mesh.aabbMax.val;
  head.matSetCount = mesh.materialSets.size();
  head.secCount = head.matSetCount + 4 + mesh.surfaces.size();
  head.secSizes.reserve(head.secCount);

  /* Lengths of padding to insert while writing */
  std::vector<size_t> paddingSizes;
  paddingSizes.reserve(head.secCount);

  /* Build material sets */
  std::vector<MaterialSet> matSets;
  matSets.reserve(mesh.materialSets.size());

  for (const std::vector<Material>& mset : mesh.materialSets) {
    matSets.emplace_back();
    MaterialSet& targetMSet = matSets.back();

    size_t endOff = 0;
    for (const Material& mat : mset) {
      ++targetMSet.materialCount;
      targetMSet.materials.emplace_back(mat);
      targetMSet.materials.back().binarySize(endOff);
      targetMSet.materialEndOffs.push_back(endOff);
    }

    size_t secSz = 0;
    targetMSet.binarySize(secSz);
    size_t secSz32 = ROUND_UP_32(secSz);
    head.secSizes.push_back(secSz32);
    paddingSizes.push_back(secSz32 - secSz);
  }

  hecl::blender::HMDLBuffers bufs = mesh.getHMDLBuffers(false, poolSkinIndex);

  /* Metadata */
  size_t secSz = 0;
  bufs.m_meta.binarySize(secSz);
  size_t secSz32 = ROUND_UP_32(secSz);
  if (secSz32 == 0)
    secSz32 = 32;
  head.secSizes.push_back(secSz32);
  paddingSizes.push_back(secSz32 - secSz);

  /* VBO */
  secSz = bufs.m_vboSz;
  secSz32 = ROUND_UP_32(secSz);
  if (secSz32 == 0)
    secSz32 = 32;
  head.secSizes.push_back(secSz32);
  paddingSizes.push_back(secSz32 - secSz);

  /* IBO */
  secSz = bufs.m_iboSz;
  secSz32 = ROUND_UP_32(secSz);
  if (secSz32 == 0)
    secSz32 = 32;
  head.secSizes.push_back(secSz32);
  paddingSizes.push_back(secSz32 - secSz);

  /* Surface index */
  std::vector<size_t> surfEndOffs;
  surfEndOffs.reserve(bufs.m_surfaces.size());
  secSz = bufs.m_surfaces.size() * 4 + 4;
  secSz32 = ROUND_UP_32(secSz);
  if (secSz32 == 0)
    secSz32 = 32;
  head.secSizes.push_back(secSz32);
  paddingSizes.push_back(secSz32 - secSz);

  /* Surfaces */
  size_t endOff = 0;
  for (const hecl::blender::HMDLBuffers::Surface& surf : bufs.m_surfaces) {
    (void)surf;
    head.secSizes.push_back(64);
    paddingSizes.push_back(0);
    endOff += 64;
    surfEndOffs.push_back(endOff);
  }

  /* Write sections */
  athena::io::FileWriter writer(outPath.getAbsolutePath());
  head.write(writer);
  std::vector<size_t>::const_iterator padIt = paddingSizes.cbegin();

  /* Material Sets */
  for (const MaterialSet& mset : matSets) {
    mset.write(writer);
    writer.fill(atUint8(0), *padIt);
    ++padIt;
  }

  /* Metadata */
  bufs.m_meta.write(writer);
  writer.fill(atUint8(0), *padIt);
  ++padIt;

  /* VBO */
  writer.writeUBytes(bufs.m_vboData.get(), bufs.m_vboSz);
  writer.fill(atUint8(0), *padIt);
  ++padIt;

  /* IBO */
  writer.writeUBytes(bufs.m_iboData.get(), bufs.m_iboSz);
  writer.fill(atUint8(0), *padIt);
  ++padIt;

  /* Surface index */
  writer.writeUint32Big(surfEndOffs.size());
  for (size_t off : surfEndOffs)
    writer.writeUint32Big(off);
  writer.fill(atUint8(0), *padIt);
  ++padIt;

  /* Surfaces */
  for (const hecl::blender::HMDLBuffers::Surface& surf : bufs.m_surfaces) {
    const Mesh::Surface& osurf = surf.m_origSurf;

    SurfaceHeader header;
    header.centroid = osurf.centroid;
    header.matIdx = osurf.materialIdx;
    header.reflectionNormal = osurf.reflectionNormal;
    header.idxStart = surf.m_start;
    header.idxCount = surf.m_count;
    header.skinMtxBankIdx = osurf.skinBankIdx;
    header.write(writer);

    writer.fill(atUint8(0), *padIt);
    ++padIt;
  }

  /* Ensure final surface's alignment writes zeros */
  writer.seek(-1, athena::SeekOrigin::Current);
  writer.writeUByte(0);
  writer.close();
  return true;
}

template bool WriteHMDLCMDL<DNAMP1::HMDLMaterialSet, DNACMDL::SurfaceHeader_2, 2>(
    const hecl::ProjectPath& outPath, const hecl::ProjectPath& inPath, const Mesh& mesh,
    hecl::blender::PoolSkinIndex& poolSkinIndex);

struct MaterialPool {
  std::vector<const Material*> materials;
  size_t addMaterial(const Material& mat, bool& newMat) {
    size_t ret = 0;
    newMat = false;
    for (const Material* testMat : materials) {
      if (mat == *testMat)
        return ret;
      ++ret;
    }
    materials.push_back(&mat);
    newMat = true;
    return ret;
  }
};

template <class MaterialSet, class SurfaceHeader, class MeshHeader>
bool WriteMREASecs(std::vector<std::vector<uint8_t>>& secsOut, const hecl::ProjectPath& inPath,
                   const std::vector<Mesh>& meshes, zeus::CAABox& fullAABB, std::vector<zeus::CAABox>& meshAABBs) {
  /* Build material set */
  std::vector<size_t> surfToGlobalMats;
  MaterialSet matSet;
  {
    MaterialPool matPool;

    size_t surfCount = 0;
    for (const Mesh& mesh : meshes)
      surfCount += mesh.surfaces.size();
    surfToGlobalMats.reserve(surfCount);

    size_t endOff = 0;
    std::vector<hecl::ProjectPath> texPaths;
    for (const Mesh& mesh : meshes) {
      if (mesh.materialSets.size()) {
        std::vector<size_t> meshToGlobalMats;
        meshToGlobalMats.reserve(mesh.materialSets[0].size());

        for (const Material& mat : mesh.materialSets[0]) {
          bool newMat;
          size_t idx = matPool.addMaterial(mat, newMat);
          meshToGlobalMats.push_back(idx);
          if (!newMat)
            continue;

          for (const auto& chunk : mat.chunks) {
            if (auto pass = chunk.get_if<Material::PASS>()) {
              bool found = false;
              for (const hecl::ProjectPath& ePath : texPaths) {
                if (pass->tex == ePath) {
                  found = true;
                  break;
                }
              }
              if (!found)
                texPaths.push_back(pass->tex);
            }
          }

          auto lightmapped = mat.iprops.find("retro_lightmapped");
          bool lm = lightmapped != mat.iprops.cend() && lightmapped->second != 0;

          matSet.materials.emplace_back(mat, texPaths, mesh.colorLayerCount, lm, false);

          matSet.materials.back().binarySize(endOff);
          matSet.head.addMaterialEndOff(endOff);
        }

        for (const Mesh::Surface& surf : mesh.surfaces)
          surfToGlobalMats.push_back(meshToGlobalMats[surf.materialIdx]);
      }
    }
    for (const hecl::ProjectPath& path : texPaths)
      matSet.head.addTexture(path);

    size_t secSz = 0;
    matSet.binarySize(secSz);
    secsOut.emplace_back(secSz, 0);
    athena::io::MemoryWriter w(secsOut.back().data(), secsOut.back().size());
    matSet.write(w);
  }

  /* Iterate meshes */
  auto matIt = surfToGlobalMats.cbegin();
  for (const Mesh& mesh : meshes) {
    zeus::CTransform meshXf(mesh.sceneXf.val.data());
    meshXf.basis.transpose();

    /* Header */
    {
      MeshHeader meshHeader = {};
      meshHeader.visorFlags.setFromBlenderProps(mesh.customProps);
      memmove(meshHeader.xfMtx, mesh.sceneXf.val.data(), 48);

      zeus::CAABox aabb(zeus::CVector3f(mesh.aabbMin), zeus::CVector3f(mesh.aabbMax));
      aabb = aabb.getTransformedAABox(meshXf);
      meshAABBs.push_back(aabb);
      fullAABB.accumulateBounds(aabb);
      meshHeader.aabb[0] = aabb.min;
      meshHeader.aabb[1] = aabb.max;

      size_t secSz = 0;
      meshHeader.binarySize(secSz);
      secsOut.emplace_back(secSz, 0);
      athena::io::MemoryWriter w(secsOut.back().data(), secsOut.back().size());
      meshHeader.write(w);
    }

    std::vector<size_t> surfEndOffs;
    surfEndOffs.reserve(mesh.surfaces.size());
    size_t endOff = 0;
    auto smatIt = matIt;
    for (const Mesh::Surface& surf : mesh.surfaces) {
      const typename MaterialSet::Material::VAFlags& vaFlags = matSet.materials.at(*smatIt++).getVAFlags();
      size_t vertSz = vaFlags.vertDLSize();

      endOff += 96;
      for (auto it = surf.verts.cbegin(); it != surf.verts.cend();) {
        atUint16 vertCount = 0;
        auto itEnd = surf.verts.cend();
        for (auto it2 = it; it2 != surf.verts.cend(); ++it2, ++vertCount)
          if (it2->iPos == 0xffffffff) {
            if (vertCount == 3) {
              /* All primitives here on out are triangles */
              vertCount = atUint16((surf.verts.cend() - it + 1) * 3 / 4);
              break;
            }
            itEnd = it2;
            break;
          }
        endOff += 3 + vertSz * vertCount;
        if (itEnd == surf.verts.cend())
          break;
        it = itEnd + 1;
      }
      endOff = ROUND_UP_32(endOff);
      surfEndOffs.push_back(endOff);
    }

    /* Positions */
    {
      size_t secSz = ROUND_UP_32(mesh.pos.size() * 12);
      if (secSz == 0)
        secSz = 32;
      secsOut.emplace_back(secSz, 0);
      athena::io::MemoryWriter w(secsOut.back().data(), secsOut.back().size());
      for (const hecl::blender::Vector3f& v : mesh.pos) {
        zeus::CVector3f preXfPos = meshXf * zeus::CVector3f(v);
        w.writeVec3fBig(preXfPos);
      }
    }

    /* Normals */
    {
      size_t secSz = ROUND_UP_32(mesh.norm.size() * 6);
      if (secSz == 0)
        secSz = 32;
      secsOut.emplace_back(secSz, 0);
      athena::io::MemoryWriter w(secsOut.back().data(), secsOut.back().size());
      for (const hecl::blender::Vector3f& v : mesh.norm) {
        zeus::CVector3f preXfNorm = (meshXf.basis * zeus::CVector3f(v)).normalized();
        for (int i = 0; i < 3; ++i) {
          int tmpV = int(preXfNorm[i] * 16384.f);
          tmpV = zeus::clamp(-32768, tmpV, 32767);
          w.writeInt16Big(atInt16(tmpV));
        }
      }
    }

    /* Colors */
    {
      size_t secSz = ROUND_UP_32(mesh.color.size() * 4);
      if (secSz == 0)
        secSz = 32;
      secsOut.emplace_back(secSz, 0);
      athena::io::MemoryWriter w(secsOut.back().data(), secsOut.back().size());
      for (const hecl::blender::Vector3f& v : mesh.color) {
        zeus::CColor col((zeus::CVector4f(zeus::CVector3f(v))));
        col.writeRGBA8(w);
      }
    }

    /* UVs */
    {
      size_t secSz = ROUND_UP_32(mesh.uv.size() * 8);
      if (secSz == 0)
        secSz = 32;
      secsOut.emplace_back(secSz, 0);
      athena::io::MemoryWriter w(secsOut.back().data(), secsOut.back().size());
      for (const hecl::blender::Vector2f& v : mesh.uv)
        w.writeVec2fBig(v.val);
    }

    /* LUVs */
    {
      size_t secSz = ROUND_UP_32(mesh.luv.size() * 4);
      if (secSz == 0)
        secSz = 32;
      secsOut.emplace_back(secSz, 0);
      athena::io::MemoryWriter w(secsOut.back().data(), secsOut.back().size());
      for (const hecl::blender::Vector2f& v : mesh.luv) {
        for (int i = 0; i < 2; ++i) {
          int tmpV = int(v.val.simd[i] * 32768.f);
          tmpV = zeus::clamp(-32768, tmpV, 32767);
          w.writeInt16Big(atInt16(tmpV));
        }
      }
    }

    /* Surface index */
    {
      secsOut.emplace_back((surfEndOffs.size() + 1) * 4, 0);
      athena::io::MemoryWriter w(secsOut.back().data(), secsOut.back().size());
      w.writeUint32Big(surfEndOffs.size());
      for (size_t off : surfEndOffs)
        w.writeUint32Big(off);
    }

    /* Surfaces */
    GX::Primitive prim = GX::TRIANGLES;
    if (mesh.topology == hecl::HMDLTopology::Triangles)
      prim = GX::TRIANGLES;
    else if (mesh.topology == hecl::HMDLTopology::TriStrips)
      prim = GX::TRIANGLESTRIP;
    else
      LogDNACommon.report(logvisor::Fatal, fmt("unrecognized mesh output mode"));
    auto surfEndOffIt = surfEndOffs.begin();
    size_t lastEndOff = 0;
    for (const Mesh::Surface& surf : mesh.surfaces) {
      size_t matIdx = *matIt++;
      const typename MaterialSet::Material& mat = matSet.materials.at(matIdx);
      const typename MaterialSet::Material::VAFlags& vaFlags = mat.getVAFlags();

      SurfaceHeader header;
      header.centroid = meshXf * zeus::CVector3f(surf.centroid);
      header.matIdx = matIdx;
      uint32_t dlSize = uint32_t(*surfEndOffIt - lastEndOff - 96);
      header.dlSize = dlSize | 0x80000000;
      lastEndOff = *surfEndOffIt++;
      header.reflectionNormal = (meshXf.basis * zeus::CVector3f(surf.reflectionNormal)).normalized();
      header.aabbSz = 24;
      zeus::CAABox aabb(zeus::CVector3f(surf.aabbMin), zeus::CVector3f(surf.aabbMax));
      aabb = aabb.getTransformedAABox(meshXf);
      header.aabb[0] = aabb.min;
      header.aabb[1] = aabb.max;

      size_t secSz = 0;
      header.binarySize(secSz);
      secSz += dlSize;
      secSz = ROUND_UP_32(secSz);
      secsOut.emplace_back(secSz, 0);
      athena::io::MemoryWriter w(secsOut.back().data(), secsOut.back().size());
      header.write(w);

      GX::Primitive usePrim = prim;
      for (auto it = surf.verts.cbegin(); it != surf.verts.cend();) {
        atUint16 vertCount = 0;
        auto itEnd = surf.verts.cend();
        for (auto it2 = it; it2 != surf.verts.cend(); ++it2, ++vertCount)
          if (it2->iPos == 0xffffffff) {
            if (vertCount == 3) {
              /* All primitives here on out are triangles */
              usePrim = GX::TRIANGLES;
              vertCount = atUint16((surf.verts.cend() - it + 1) * 3 / 4);
              break;
            }
            itEnd = it2;
            break;
          }

        /* VAT1 = short normals, float UVs
         * VAT2 = short normals, short UVs */
        w.writeUByte(usePrim | (mat.flags.lightmapUVArray() ? 0x2 : 0x1));
        w.writeUint16Big(vertCount);

        for (auto it2 = it; it2 != itEnd; ++it2) {
          const Mesh::Surface::Vert& vert = *it2;
          if (vert.iPos == 0xffffffff)
            continue;
          atUint32 skinIdx = vert.iBankSkin * 3;
          WriteDLVal(w, vaFlags.pnMatIdx(), skinIdx);
          WriteDLVal(w, vaFlags.tex0MatIdx(), skinIdx);
          WriteDLVal(w, vaFlags.tex1MatIdx(), skinIdx);
          WriteDLVal(w, vaFlags.tex2MatIdx(), skinIdx);
          WriteDLVal(w, vaFlags.tex3MatIdx(), skinIdx);
          WriteDLVal(w, vaFlags.tex4MatIdx(), skinIdx);
          WriteDLVal(w, vaFlags.tex5MatIdx(), skinIdx);
          WriteDLVal(w, vaFlags.tex6MatIdx(), skinIdx);
          WriteDLVal(w, vaFlags.position(), vert.iPos);
          WriteDLVal(w, vaFlags.normal(), vert.iNorm);
          WriteDLVal(w, vaFlags.color0(), vert.iColor[0]);
          WriteDLVal(w, vaFlags.color1(), vert.iColor[1]);
          WriteDLVal(w, vaFlags.tex0(), vert.iUv[0]);
          WriteDLVal(w, vaFlags.tex1(), vert.iUv[1]);
          WriteDLVal(w, vaFlags.tex2(), vert.iUv[2]);
          WriteDLVal(w, vaFlags.tex3(), vert.iUv[3]);
          WriteDLVal(w, vaFlags.tex4(), vert.iUv[4]);
          WriteDLVal(w, vaFlags.tex5(), vert.iUv[5]);
          WriteDLVal(w, vaFlags.tex6(), vert.iUv[6]);
        }

        if (itEnd == surf.verts.cend())
          break;
        it = itEnd + 1;
      }
    }
  }

  return true;
}

template bool WriteMREASecs<DNAMP1::MaterialSet, DNACMDL::SurfaceHeader_1, DNAMP1::MREA::MeshHeader>(
    std::vector<std::vector<uint8_t>>& secsOut, const hecl::ProjectPath& inPath, const std::vector<Mesh>& meshes,
    zeus::CAABox& fullAABB, std::vector<zeus::CAABox>& meshAABBs);

template <class MaterialSet, class SurfaceHeader, class MeshHeader>
bool WriteHMDLMREASecs(std::vector<std::vector<uint8_t>>& secsOut, const hecl::ProjectPath& inPath,
                       const std::vector<Mesh>& meshes, zeus::CAABox& fullAABB, std::vector<zeus::CAABox>& meshAABBs) {
  /* Build material set */
  std::vector<size_t> surfToGlobalMats;
  {
    MaterialPool matPool;

    size_t surfCount = 0;
    for (const Mesh& mesh : meshes)
      surfCount += mesh.surfaces.size();
    surfToGlobalMats.reserve(surfCount);

    MaterialSet matSet = {};
    size_t endOff = 0;
    for (const Mesh& mesh : meshes) {
      if (mesh.materialSets.size()) {
        std::vector<size_t> meshToGlobalMats;
        meshToGlobalMats.reserve(mesh.materialSets[0].size());

        for (const Material& mat : mesh.materialSets[0]) {
          bool newMat;
          size_t idx = matPool.addMaterial(mat, newMat);
          meshToGlobalMats.push_back(idx);
          if (!newMat)
            continue;

          ++matSet.materialCount;
          matSet.materials.emplace_back(mat);
          matSet.materials.back().binarySize(endOff);
          matSet.materialEndOffs.push_back(endOff);
        }

        for (const Mesh::Surface& surf : mesh.surfaces)
          surfToGlobalMats.push_back(meshToGlobalMats[surf.materialIdx]);
      }
    }

    size_t secSz = 0;
    matSet.binarySize(secSz);
    secsOut.emplace_back(secSz, 0);
    athena::io::MemoryWriter w(secsOut.back().data(), secsOut.back().size());
    matSet.write(w);
  }

  /* Iterate meshes */
  auto matIt = surfToGlobalMats.cbegin();
  for (const Mesh& mesh : meshes) {
    zeus::CTransform meshXf(mesh.sceneXf.val.data());
    meshXf.basis.transpose();

    /* Header */
    {
      MeshHeader meshHeader = {};
      meshHeader.visorFlags.setFromBlenderProps(mesh.customProps);
      memmove(meshHeader.xfMtx, mesh.sceneXf.val.data(), 48);

      zeus::CAABox aabb(zeus::CVector3f(mesh.aabbMin), zeus::CVector3f(mesh.aabbMax));
      aabb = aabb.getTransformedAABox(meshXf);
      meshAABBs.push_back(aabb);
      fullAABB.accumulateBounds(aabb);
      meshHeader.aabb[0] = aabb.min;
      meshHeader.aabb[1] = aabb.max;

      size_t secSz = 0;
      meshHeader.binarySize(secSz);
      secsOut.emplace_back(secSz, 0);
      athena::io::MemoryWriter w(secsOut.back().data(), secsOut.back().size());
      meshHeader.write(w);
    }

    hecl::blender::PoolSkinIndex poolSkinIndex;
    hecl::blender::HMDLBuffers bufs = mesh.getHMDLBuffers(true, poolSkinIndex);

    std::vector<size_t> surfEndOffs;
    surfEndOffs.reserve(bufs.m_surfaces.size());
    size_t endOff = 0;
    for (const hecl::blender::HMDLBuffers::Surface& surf : bufs.m_surfaces) {
      (void)surf;
      endOff += 96;
      surfEndOffs.push_back(endOff);
    }

    /* Metadata */
    {
      size_t secSz = 0;
      bufs.m_meta.binarySize(secSz);
      secsOut.emplace_back(secSz, 0);
      athena::io::MemoryWriter w(secsOut.back().data(), secsOut.back().size());
      bufs.m_meta.write(w);
    }

    /* VBO */
    {
      secsOut.emplace_back(bufs.m_vboSz, 0);
      athena::io::MemoryWriter w(secsOut.back().data(), secsOut.back().size());
      w.writeUBytes(bufs.m_vboData.get(), bufs.m_vboSz);
    }

    /* IBO */
    {
      secsOut.emplace_back(bufs.m_iboSz, 0);
      athena::io::MemoryWriter w(secsOut.back().data(), secsOut.back().size());
      w.writeUBytes(bufs.m_iboData.get(), bufs.m_iboSz);
    }

    /* Surface index */
    {
      secsOut.emplace_back((surfEndOffs.size() + 1) * 4, 0);
      athena::io::MemoryWriter w(secsOut.back().data(), secsOut.back().size());
      w.writeUint32Big(surfEndOffs.size());
      for (size_t off : surfEndOffs)
        w.writeUint32Big(off);
    }

    /* Surfaces */
    for (const hecl::blender::HMDLBuffers::Surface& surf : bufs.m_surfaces) {
      const Mesh::Surface& osurf = surf.m_origSurf;

      SurfaceHeader header;
      header.centroid = meshXf * zeus::CVector3f(osurf.centroid);
      header.matIdx = *matIt++;
      header.reflectionNormal = (meshXf.basis * zeus::CVector3f(osurf.reflectionNormal)).normalized();
      header.idxStart = surf.m_start;
      header.idxCount = surf.m_count;
      header.skinMtxBankIdx = osurf.skinBankIdx;

      header.aabbSz = 24;
      zeus::CAABox aabb(zeus::CVector3f(surf.m_origSurf.aabbMin), zeus::CVector3f(surf.m_origSurf.aabbMax));
      aabb = aabb.getTransformedAABox(meshXf);
      header.aabb[0] = aabb.min;
      header.aabb[1] = aabb.max;

      size_t secSz = 0;
      header.binarySize(secSz);
      secsOut.emplace_back(secSz, 0);
      athena::io::MemoryWriter w(secsOut.back().data(), secsOut.back().size());
      header.write(w);
    }
  }

  return true;
}

template bool WriteHMDLMREASecs<DNAMP1::HMDLMaterialSet, DNACMDL::SurfaceHeader_2, DNAMP1::MREA::MeshHeader>(
    std::vector<std::vector<uint8_t>>& secsOut, const hecl::ProjectPath& inPath, const std::vector<Mesh>& meshes,
    zeus::CAABox& fullAABB, std::vector<zeus::CAABox>& meshAABBs);

template <>
void SurfaceHeader_1::Enumerate<BigDNA::Read>(typename Read::StreamT& reader) {
  /* centroid */
  centroid = reader.readVec3fBig();
  /* matIdx */
  matIdx = reader.readUint32Big();
  /* dlSize */
  dlSize = reader.readUint32Big();
  /* idxStart */
  idxStart = reader.readUint32Big();
  /* idxCount */
  idxCount = reader.readUint32Big();
  /* aabbSz */
  aabbSz = reader.readUint32Big();
  /* reflectionNormal */
  reflectionNormal = reader.readVec3fBig();
  /* aabb */
  size_t remAABB = aabbSz;
  if (remAABB >= 24) {
    aabb[0] = reader.readVec3fBig();
    aabb[1] = reader.readVec3fBig();
    remAABB -= 24;
  }
  reader.seek(remAABB, athena::SeekOrigin::Current);
  /* align */
  reader.seekAlign32();
}

template <>
void SurfaceHeader_1::Enumerate<BigDNA::Write>(typename Write::StreamT& writer) {
  /* centroid */
  writer.writeVec3fBig(centroid);
  /* matIdx */
  writer.writeUint32Big(matIdx);
  /* dlSize */
  writer.writeUint32Big(dlSize);
  /* idxStart */
  writer.writeUint32Big(idxStart);
  /* idxCount */
  writer.writeUint32Big(idxCount);
  /* aabbSz */
  writer.writeUint32Big(aabbSz ? 24 : 0);
  /* reflectionNormal */
  writer.writeVec3fBig(reflectionNormal);
  /* aabb */
  if (aabbSz) {
    writer.writeVec3fBig(aabb[0]);
    writer.writeVec3fBig(aabb[1]);
  }
  /* align */
  writer.seekAlign32();
}

template <>
void SurfaceHeader_1::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& s) {
  s += (aabbSz ? 24 : 0);
  s += 44;
  s = (s + 31) & ~31;
}

template <>
void SurfaceHeader_2::Enumerate<BigDNA::Read>(typename Read::StreamT& reader) {
  /* centroid */
  centroid = reader.readVec3fBig();
  /* matIdx */
  matIdx = reader.readUint32Big();
  /* dlSize */
  dlSize = reader.readUint32Big();
  /* idxStart */
  idxStart = reader.readUint32Big();
  /* idxCount */
  idxCount = reader.readUint32Big();
  /* aabbSz */
  aabbSz = reader.readUint32Big();
  /* reflectionNormal */
  reflectionNormal = reader.readVec3fBig();
  /* skinMtxBankIdx */
  skinMtxBankIdx = reader.readInt16Big();
  /* surfaceGroup */
  surfaceGroup = reader.readUint16Big();
  /* aabb */
  size_t remAABB = aabbSz;
  if (remAABB >= 24) {
    aabb[0] = reader.readVec3fBig();
    aabb[1] = reader.readVec3fBig();
    remAABB -= 24;
  }
  reader.seek(remAABB, athena::SeekOrigin::Current);
  /* align */
  reader.seekAlign32();
}

template <>
void SurfaceHeader_2::Enumerate<BigDNA::Write>(typename Write::StreamT& writer) {
  /* centroid */
  writer.writeVec3fBig(centroid);
  /* matIdx */
  writer.writeUint32Big(matIdx);
  /* dlSize */
  writer.writeUint32Big(dlSize);
  /* idxStart */
  writer.writeUint32Big(idxStart);
  /* idxCount */
  writer.writeUint32Big(idxCount);
  /* aabbSz */
  writer.writeUint32Big(aabbSz ? 24 : 0);
  /* reflectionNormal */
  writer.writeVec3fBig(reflectionNormal);
  /* skinMtxBankIdx */
  writer.writeInt16Big(skinMtxBankIdx);
  /* surfaceGroup */
  writer.writeUint16Big(surfaceGroup);
  /* aabb */
  if (aabbSz) {
    writer.writeVec3fBig(aabb[0]);
    writer.writeVec3fBig(aabb[1]);
  }
  /* align */
  writer.seekAlign32();
}

template <>
void SurfaceHeader_2::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& s) {
  s += (aabbSz ? 24 : 0);
  s += 48;
  s = (s + 31) & ~31;
}

template <>
void SurfaceHeader_3::Enumerate<BigDNA::Read>(typename Read::StreamT& reader) {
  /* centroid */
  centroid = reader.readVec3fBig();
  /* matIdx */
  matIdx = reader.readUint32Big();
  /* dlSize */
  dlSize = reader.readUint32Big();
  /* idxStart */
  idxStart = reader.readUint32Big();
  /* idxCount */
  idxCount = reader.readUint32Big();
  /* aabbSz */
  aabbSz = reader.readUint32Big();
  /* reflectionNormal */
  reflectionNormal = reader.readVec3fBig();
  /* skinMtxBankIdx */
  skinMtxBankIdx = reader.readInt16Big();
  /* surfaceGroup */
  surfaceGroup = reader.readUint16Big();
  /* aabb */
  size_t remAABB = aabbSz;
  if (remAABB >= 24) {
    aabb[0] = reader.readVec3fBig();
    aabb[1] = reader.readVec3fBig();
    remAABB -= 24;
  }
  reader.seek(remAABB, athena::SeekOrigin::Current);
  /* unk3 */
  unk3 = reader.readUByte();
  /* align */
  reader.seekAlign32();
}

template <>
void SurfaceHeader_3::Enumerate<BigDNA::Write>(typename Write::StreamT& writer) {
  /* centroid */
  writer.writeVec3fBig(centroid);
  /* matIdx */
  writer.writeUint32Big(matIdx);
  /* dlSize */
  writer.writeUint32Big(dlSize);
  /* idxStart */
  writer.writeUint32Big(idxStart);
  /* idxCount */
  writer.writeUint32Big(idxCount);
  /* aabbSz */
  writer.writeUint32Big(aabbSz ? 24 : 0);
  /* reflectionNormal */
  writer.writeVec3fBig(reflectionNormal);
  /* skinMtxBankIdx */
  writer.writeInt16Big(skinMtxBankIdx);
  /* surfaceGroup */
  writer.writeUint16Big(surfaceGroup);
  /* aabb */
  if (aabbSz) {
    writer.writeVec3fBig(aabb[0]);
    writer.writeVec3fBig(aabb[1]);
  }
  /* unk3 */
  writer.writeUByte(unk3);
  /* align */
  writer.seekAlign32();
}

template <>
void SurfaceHeader_3::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& s) {
  s += (aabbSz ? 24 : 0);
  s += 49;
  s = (s + 31) & ~31;
}

} // namespace DataSpec::DNACMDL
