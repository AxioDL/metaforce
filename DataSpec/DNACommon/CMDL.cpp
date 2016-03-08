#include "CMDL.hpp"
#include "../DNAMP1/CMDLMaterials.hpp"
#include "../DNAMP1/CSKR.hpp"
#include "../DNAMP2/CMDLMaterials.hpp"
#include "../DNAMP2/CSKR.hpp"
#include "../DNAMP3/CMDLMaterials.hpp"
#include "../DNAMP3/CSKR.hpp"

namespace DataSpec
{
namespace DNACMDL
{

template <class MaterialSet>
void GetVertexAttributes(const MaterialSet& matSet,
                         std::vector<VertexAttributes>& attributesOut)
{
    attributesOut.clear();
    attributesOut.reserve(matSet.materials.size());

    for (const typename MaterialSet::Material& mat : matSet.materials)
    {
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
void ReadMaterialSetToBlender_1_2(hecl::BlenderConnection::PyOutStream& os,
                                  const MaterialSet& matSet,
                                  const PAKRouter& pakRouter,
                                  const typename PAKRouter::EntryType& entry,
                                  unsigned setIdx)
{
    /* Texmaps */
    os << "texmap_list = []\n";
    for (const UniqueID32& tex : matSet.head.textureIDs)
    {
        std::string texName = pakRouter.getBestEntryName(tex);
        const nod::Node* node;
        const typename PAKRouter::EntryType* texEntry = pakRouter.lookupEntry(tex, &node);
        hecl::ProjectPath txtrPath = pakRouter.getWorking(texEntry);
        if (txtrPath.getPathType() == hecl::ProjectPath::Type::None)
        {
            PAKEntryReadStream rs = texEntry->beginReadStream(*node);
            TXTR::Extract(rs, txtrPath);
        }
        hecl::SystemString resPath = pakRouter.getResourceRelativePath(entry, tex);
        hecl::SystemUTF8View resPathView(resPath);
        os.format("if '%s' in bpy.data.textures:\n"
                  "    image = bpy.data.images['%s']\n"
                  "    texture = bpy.data.textures[image.name]\n"
                  "else:\n"
                  "    image = bpy.data.images.load('''//%s''')\n"
                  "    image.name = '%s'\n"
                  "    texture = bpy.data.textures.new(image.name, 'IMAGE')\n"
                  "    texture.image = image\n"
                  "texmap_list.append(texture)\n"
                  "\n", texName.c_str(), texName.c_str(),
                  resPathView.str().c_str(), texName.c_str());
    }

    unsigned m=0;
    for (const typename MaterialSet::Material& mat : matSet.materials)
    {
        MaterialSet::ConstructMaterial(os, mat, setIdx, m++);
        os << "materials.append(new_material)\n";
    }
}

template <class PAKRouter, class MaterialSet>
void ReadMaterialSetToBlender_3(hecl::BlenderConnection::PyOutStream& os,
                                const MaterialSet& matSet,
                                const PAKRouter& pakRouter,
                                const typename PAKRouter::EntryType& entry,
                                unsigned setIdx)
{
    unsigned m=0;
    for (const typename MaterialSet::Material& mat : matSet.materials)
    {
        MaterialSet::ConstructMaterial(os, pakRouter, entry, mat, setIdx, m++);
        os << "materials.append(new_material)\n";
    }
}

template void ReadMaterialSetToBlender_3<PAKRouter<DNAMP3::PAKBridge>, DNAMP3::MaterialSet>
(hecl::BlenderConnection::PyOutStream& os,
 const DNAMP3::MaterialSet& matSet,
 const PAKRouter<DNAMP3::PAKBridge>& pakRouter,
 const PAKRouter<DNAMP3::PAKBridge>::EntryType& entry,
 unsigned setIdx);

class DLReader
{
public:
    /* Class used for splitting verts with shared positions but different skinning matrices */
    class ExtraVertTracker
    {
        std::map<atUint16, std::vector<std::pair<atInt16, atUint16>>> m_extraVerts;
        atUint16 m_maxBasePos = 0;
        atUint16 m_nextOverPos = 1;
    public:
        atInt16 addPosSkinPair(atUint16 pos, atInt16 skin)
        {
            m_maxBasePos = std::max(m_maxBasePos, pos);
            auto search = m_extraVerts.find(pos);
            if (search == m_extraVerts.end())
            {
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

        template<class RigPair>
        void sendAdditionalVertsToBlender(hecl::BlenderConnection::PyOutStream& os,
                                          const RigPair& rp) const
        {
            atUint32 nextVert = 1;
            while (nextVert < m_nextOverPos)
            {
                for (const std::pair<atUint16, std::vector<std::pair<atInt16, atUint16>>>& ev : m_extraVerts)
                {
                    for (const std::pair<atInt16, atUint16>& se : ev.second)
                    {
                        if (se.second == nextVert)
                        {
                            os.format("bm.verts.ensure_lookup_table()\n"
                                      "orig_vert = bm.verts[%u]\n"
                                      "vert = bm.verts.new(orig_vert.co)\n",
                                      ev.first);
                            rp.first->weightVertex(os, *rp.second, se.first);
                            ++nextVert;
                        }
                    }
                }
            }
        }

        atUint16 lookupVertIdx(atUint16 pos, atInt16 skin) const
        {
            auto search = m_extraVerts.find(pos);
            if (search == m_extraVerts.end())
                return -1;
            const std::vector<std::pair<atInt16, atUint16>>& vertTrack = search->second;
            if (vertTrack.front().first == skin)
                return pos;
            for (auto it=vertTrack.begin()+1 ; it!=vertTrack.end() ; ++it)
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
    atUint16 readVal(GX::AttrType type)
    {
        atUint16 retval = 0;
        switch (type)
        {
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
        default: break;
        }
        return retval;
    }
public:
    DLReader(const VertexAttributes& va, std::unique_ptr<atUint8[]>&& dl,
             size_t dlSize, ExtraVertTracker& evt, const atInt16* bankIn=nullptr)
    : m_va(va), m_dl(std::move(dl)), m_dlSize(dlSize), m_evt(evt), m_bankIn(bankIn)
    {
        m_cur = m_dl.get();
    }

    operator bool()
    {
        return ((m_cur - m_dl.get()) < intptr_t(m_dlSize)) && *m_cur;
    }

    GX::Primitive readPrimitive()
    {
        return GX::Primitive(*m_cur++ & 0xf8);
    }

    GX::Primitive readPrimitiveAndVat(unsigned& vatOut)
    {
        atUint8 val = *m_cur++;
        vatOut = val & 0x7;
        return GX::Primitive(val & 0xf8);
    }

    atUint16 readVertCount()
    {
        atUint16 retval = hecl::SBig(*(atUint16*)m_cur);
        m_cur += 2;
        return retval;
    }

    struct DLPrimVert
    {
        atUint16 pos = 0;
        atUint16 norm = 0;
        atUint16 color[2] = {0};
        atUint16 uvs[7] = {0};
        atUint8 pnMtxIdx = 0;
        atUint8 texMtxIdx[7] = {0};
    };

    DLPrimVert readVert(bool peek=false)
    {
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
        if (m_bankIn)
        {
            atUint16 posIdx = readVal(m_va.pos);
            atUint8 mtxIdx = retval.pnMtxIdx / 3;
            atInt16 skinIdx = -1;
            if (mtxIdx < 10)
                skinIdx = m_bankIn[mtxIdx];
            retval.pos = m_evt.lookupVertIdx(posIdx, skinIdx);
        }
        else
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

    void preReadMaxIdxs(DLPrimVert& out)
    {
        atUint8* bakCur = m_cur;
        while (*this)
        {
            readPrimitive();
            atUint16 vc = readVertCount();
            for (atUint16 v=0 ; v<vc ; ++v)
            {
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

    void preReadMaxIdxs(DLPrimVert& out,
                        std::vector<atInt16>& skinOut)
    {
        atUint8* bakCur = m_cur;
        while (*this)
        {
            readPrimitive();
            atUint16 vc = readVertCount();
            for (atUint16 v=0 ; v<vc ; ++v)
            {
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

                atInt16 skinIdx = m_bankIn[pnMtxVal/3];
                skinOut[posVal] = m_evt.addPosSkinPair(posVal, skinIdx);
            }
        }
        m_cur = bakCur;
    }
};

void InitGeomBlenderContext(hecl::BlenderConnection::PyOutStream& os,
                            const hecl::ProjectPath& masterShaderPath)
{
    os << "# Using 'Blender Game'\n"
          "bpy.context.scene.render.engine = 'BLENDER_GAME'\n"
          "\n"
          "# Clear Scene\n"
          "for ob in bpy.data.objects:\n"
          "    if ob.type != 'LAMP':\n"
          "        bpy.context.scene.objects.unlink(ob)\n"
          "        bpy.data.objects.remove(ob)\n"
          "\n"
          "def loop_from_facevert(bm, face, vert_idx):\n"
          "    for loop in face.loops:\n"
          "        if loop.vert[bm.verts.layers.int['CMDLOriginalPosIdxs']] == vert_idx:\n"
          "            return loop\n"
          "\n"
          "def add_triangle(bm, vert_seq, vert_indices, norm_seq, norm_indices, mat_nr, od_list):\n"
          "    if len(set(vert_indices)) != 3:\n"
          "        return None, None\n"
          "\n"
          "    ret_mesh = bm\n"
          "    vert_seq.ensure_lookup_table()\n"
          "    verts = [vert_seq[i] for i in vert_indices]\n"
          "\n"
          "    # Make the face\n"
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
          "            bmesh.ops.delete(od_entry['bm'], geom=od_entry['bm'].faces, context=3)\n"
          "            od_list.append(od_entry)\n"
          "        od_entry['bm'].verts.ensure_lookup_table()\n"
          "        verts = [od_entry['bm'].verts[i] for i in vert_indices]\n"
          "        face = od_entry['bm'].faces.get(verts)\n"
          "        if face is None:\n"
          "            face = od_entry['bm'].faces.new(verts)\n"
          "        else: # Probably a double-sided surface\n"
          "            face = face.copy()\n"
          "            for i in range(3):\n"
          "                face.verts[i].co = verts[i].co\n"
          "        ret_mesh = od_entry['bm']\n"
          "\n"
          "    elif face is not None: # Same material, probably double-sided\n"
          "        face = face.copy()\n"
          "        for i in range(3):\n"
          "            face.verts[i].co = verts[i].co\n"
          "\n"
          "    else: \n"
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
          "def expand_lightmap_triangle(uva, uvb, uvc):\n"
          "    result = ([uva[0],uva[1]], [uvb[0],uvb[1]], [uvc[0],uvc[1]])\n"
          "    if uva == uvb:\n"
          "        result[1][0] += 0.005\n"
          "    if uva == uvc:\n"
          "        result[2][1] -= 0.005\n"
          "    return result\n"
          "\n";

    /* Link master shader library */
    os.format("# Master shader library\n"
              "with bpy.data.libraries.load('%s', link=True, relative=True) as (data_from, data_to):\n"
              "    data_to.node_groups = data_from.node_groups\n"
              "\n", masterShaderPath.getAbsolutePathUTF8().c_str());
}

void FinishBlenderMesh(hecl::BlenderConnection::PyOutStream& os,
                       unsigned matSetCount, int meshIdx)
{
    if (meshIdx < 0)
        os.format("mesh = bpy.data.meshes.new(bpy.context.scene.name)\n"
                  "obj = bpy.data.objects.new(mesh.name, mesh)\n"
                  "obj.show_transparent = True\n"
                  "bpy.context.scene.objects.link(obj)\n"
                  "mesh.hecl_material_count = %u\n", matSetCount);
    else
        os.format("mesh = bpy.data.meshes.new(bpy.context.scene.name + '_%03d')\n"
                  "obj = bpy.data.objects.new(mesh.name, mesh)\n"
                  "obj.show_transparent = True\n"
                  "bpy.context.scene.objects.link(obj)\n"
                  "mesh.hecl_material_count = %u\n", meshIdx, matSetCount);

    os << "mesh.use_auto_smooth = True\n"
          "\n"
          "for material in materials:\n"
          "    mesh.materials.append(material)\n"
          "\n"
          "# Merge OD meshes\n"
          "for od_entry in od_list:\n"
          "    vert_dict = {}\n"
          "\n"
          "    for vert in od_entry['bm'].verts:\n"
          "        if len(vert.link_faces):\n"
          "            copy_vert = bm.verts.new(vert.co, vert)\n"
          "            vert_dict[vert[od_entry['bm'].verts.layers.int['CMDLOriginalPosIdxs']]] = copy_vert\n"
          "            copy_vert[orig_pidx_lay] = vert[od_entry['bm'].verts.layers.int['CMDLOriginalPosIdxs']]\n"
          "\n"
          "    for face in od_entry['bm'].faces:\n"
          "        merge_verts = [vert_dict[fv[od_entry['bm'].verts.layers.int['CMDLOriginalPosIdxs']]] for fv in face.verts]\n"
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
          "bm.to_mesh(mesh)\n"
          "\n"
          "# Load split normal data\n"
          "split_normals = []\n"
          "for face in bm.faces:\n"
          "    for loop in face.loops:\n"
          "        split_normals.append(norm_list[loop[orig_nidx_lay]])\n"
          "mesh.normals_split_custom_set(split_normals)\n"
          "\n"
          "bm.free()\n"
          "\n"
          "# Remove redundant materials\n"
          "present_mats = set()\n"
          "for poly in mesh.polygons:\n"
          "    present_mats.add(poly.material_index)\n"
          "for mat_idx in reversed(range(len(mesh.materials))):\n"
          "    if mat_idx not in present_mats:\n"
          "        mesh.materials.pop(index=mat_idx, update_data=True)\n"
          "\n"
          "mesh.update()\n"
          "\n";
}

template <class PAKRouter, class MaterialSet, class RigPair, class SurfaceHeader>
atUint32 ReadGeomSectionsToBlender(hecl::BlenderConnection::PyOutStream& os,
                                   athena::io::IStreamReader& reader,
                                   PAKRouter& pakRouter,
                                   const typename PAKRouter::EntryType& entry,
                                   const RigPair& rp,
                                   bool shortNormals,
                                   bool shortUVs,
                                   std::vector<VertexAttributes>& vertAttribs,
                                   int meshIdx,
                                   atUint32 secCount,
                                   atUint32 matSetCount,
                                   const atUint32* secSizes,
                                   atUint32 surfaceCount)
{
    os << "# Begin bmesh\n"
          "bm = bmesh.new()\n"
          "\n"
          "# Overdraw-tracking\n"
          "od_list = []\n"
          "\n"
          "orig_pidx_lay = bm.verts.layers.int.new('CMDLOriginalPosIdxs')\n"
          "orig_nidx_lay = bm.loops.layers.int.new('CMDLOriginalNormIdxs')\n";

    if (rp.first)
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
    for (size_t s=0 ; s<lastDlSec ; ++s)
    {
        atUint64 secStart = reader.position();
        if (s < matSecCount)
        {
            if (!s)
            {
                MaterialSet matSet;
                matSet.read(reader);
                GetVertexAttributes(matSet, vertAttribs);
            }
        }
        else
        {
            switch (s-matSecCount)
            {
            case 0:
            {
                /* Positions */
                if (SurfaceHeader::UseMatrixSkinning() && rp.first)
                    skinIndices.assign(secSizes[s] / 12, -1);
                break;
            }
            case 1:
            {
                /* Normals */
                break;
            }
            case 2:
            {
                /* Colors */
                break;
            }
            case 3:
            {
                /* Float UVs */
                break;
            }
            case 4:
            {
                if (surfaceCount)
                {
                    /* MP3 MREA case */
                    visitedDLOffsets = true;
                    lastDlSec = 4 + surfaceCount;
                }
                else
                {
                    /* Short UVs */
                    if (shortUVs)
                        break;

                    /* DL Offsets (here or next section) */
                    visitedDLOffsets = true;
                    lastDlSec = s + reader.readUint32Big() + 1;
                    break;
                }
            }
            default:
            {
                if (!visitedDLOffsets)
                {
                    visitedDLOffsets = true;
                    lastDlSec = s + reader.readUint32Big() + 1;
                    break;
                }

                /* GX Display List (surface) */
                SurfaceHeader sHead;
                sHead.read(reader);
                const atInt16* bankIn = nullptr;
                if (SurfaceHeader::UseMatrixSkinning() && rp.first)
                    bankIn = rp.first->getMatrixBank(sHead.skinMatrixBankIdx());

                /* Do max index pre-read */
                atUint32 realDlSize = secSizes[s] - (reader.position() - secStart);
                DLReader dl(vertAttribs[sHead.matIdx], reader.readUBytes(realDlSize),
                            realDlSize, extraTracker, bankIn);
                if (SurfaceHeader::UseMatrixSkinning() && rp.first)
                    dl.preReadMaxIdxs(maxIdxs, skinIndices);
                else
                    dl.preReadMaxIdxs(maxIdxs);

            }
            }
        }

        if (s < secCount - 1)
            reader.seek(secStart + secSizes[s], athena::Begin);
    }

    reader.seek(afterHeaderPos, athena::Begin);

    visitedDLOffsets = false;
    unsigned createdUVLayers = 0;
    unsigned surfIdx = 0;

    for (size_t s=0 ; s<lastDlSec ; ++s)
    {
        atUint64 secStart = reader.position();
        if (s < matSecCount)
        {
            MaterialSet matSet;
            matSet.read(reader);
            matSet.readToBlender(os, pakRouter, entry, s);
            if (!s)
                GetVertexAttributes(matSet, vertAttribs);
        }
        else
        {
            switch (s-matSecCount)
            {
            case 0:
            {
                /* Positions */
                for (size_t i=0 ; i<=maxIdxs.pos ; ++i)
                {
                    atVec3f pos = reader.readVec3fBig();
                    os.format("vert = bm.verts.new((%f,%f,%f))\n",
                              pos.vec[0], pos.vec[1], pos.vec[2]);
                    if (rp.first)
                    {
                        if (SurfaceHeader::UseMatrixSkinning() && !skinIndices.empty())
                            rp.first->weightVertex(os, *rp.second, skinIndices[i]);
                        else if (!SurfaceHeader::UseMatrixSkinning())
                            rp.first->weightVertex(os, *rp.second, i);
                    }
                }
                if (rp.first && SurfaceHeader::UseMatrixSkinning() && !skinIndices.empty())
                    extraTracker.sendAdditionalVertsToBlender(os, rp);
                break;
            }
            case 1:
            {
                /* Normals */
                os << "norm_list = []\n";
                if (shortNormals)
                {
                    size_t normCount = secSizes[s] / 6;
                    for (size_t i=0 ; i<normCount ; ++i)
                    {
                        float x = reader.readInt16Big() / 16834.0f;
                        float y = reader.readInt16Big() / 16834.0f;
                        float z = reader.readInt16Big() / 16834.0f;
                        os.format("norm_list.append((%f,%f,%f))\n",
                                  x, y, z);
                    }
                }
                else
                {
                    size_t normCount = secSizes[s] / 12;
                    for (size_t i=0 ; i<normCount ; ++i)
                    {
                        atVec3f norm = reader.readVec3fBig();
                        os.format("norm_list.append((%f,%f,%f))\n",
                                  norm.vec[0], norm.vec[1], norm.vec[2]);
                    }
                }
                break;
            }
            case 2:
            {
                /* Colors */
                break;
            }
            case 3:
            {
                /* Float UVs */
                os << "uv_list = []\n";
                size_t uvCount = secSizes[s] / 8;
                for (size_t i=0 ; i<uvCount ; ++i)
                {
                    atVec2f uv = reader.readVec2fBig();
                    os.format("uv_list.append((%f,%f))\n",
                              uv.vec[0], uv.vec[1]);
                }
                break;
            }
            case 4:
            {
                if (surfaceCount)
                {
                    /* MP3 MREA case */
                    visitedDLOffsets = true;
                }
                else
                {
                    /* Short UVs */
                    os << "suv_list = []\n";
                    if (shortUVs)
                    {
                        size_t uvCount = secSizes[s] / 4;
                        for (size_t i=0 ; i<uvCount ; ++i)
                        {
                            float x = reader.readInt16Big() / 32768.0f;
                            float y = reader.readInt16Big() / 32768.0f;
                            os.format("suv_list.append((%f,%f))\n",
                                      x, y);
                        }
                        break;
                    }

                    /* DL Offsets (here or next section) */
                    visitedDLOffsets = true;
                    break;
                }
            }
            default:
            {
                if (!visitedDLOffsets)
                {
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
                if (SurfaceHeader::UseMatrixSkinning() && rp.first)
                    bankIn = rp.first->getMatrixBank(sHead.skinMatrixBankIdx());

                os.format("materials[%u].pass_index = %u\n", sHead.matIdx, surfIdx++);
                if (matUVCount > createdUVLayers)
                {
                    for (unsigned l=createdUVLayers ; l<matUVCount ; ++l)
                        os.format("bm.loops.layers.uv.new('UV_%u')\n", l);
                    createdUVLayers = matUVCount;
                }

                atUint32 realDlSize = secSizes[s] - (reader.position() - secStart);
                DLReader dl(vertAttribs[sHead.matIdx], reader.readUBytes(realDlSize), realDlSize, extraTracker, bankIn);

                while (dl)
                {
                    GX::Primitive ptype = dl.readPrimitive();
                    atUint16 vertCount = dl.readVertCount();

                    /* First vert */
                    DLReader::DLPrimVert firstPrimVert = dl.readVert(true);

                    /* 3 Prim Verts to start */
                    int c = 0;
                    DLReader::DLPrimVert primVerts[3] =
                    {
                        dl.readVert(),
                        dl.readVert(),
                        dl.readVert()
                    };

                    if (ptype == GX::TRIANGLESTRIP)
                    {
                        atUint8 flip = 0;
                        for (int v=0 ; v<vertCount-2 ; ++v)
                        {
                            if (flip)
                            {
                                os.format("last_face, last_mesh = add_triangle(bm, bm.verts, (%u,%u,%u), norm_list, (%u,%u,%u), %u, od_list)\n",
                                          primVerts[c%3].pos,
                                          primVerts[(c+2)%3].pos,
                                          primVerts[(c+1)%3].pos,
                                          primVerts[c%3].norm,
                                          primVerts[(c+2)%3].norm,
                                          primVerts[(c+1)%3].norm,
                                          sHead.matIdx);
                                if (matUVCount)
                                {
                                    os << "if last_face is not None:\n";
                                    for (unsigned j=0 ; j<matUVCount ; ++j)
                                    {
                                        if (j==0 && matShortUVs)
                                            os.format("    uv_tri = expand_lightmap_triangle(suv_list[%u], suv_list[%u], suv_list[%u])\n"
                                                      "    loop_from_facevert(last_mesh, last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_tri[0]\n"
                                                      "    loop_from_facevert(last_mesh, last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_tri[1]\n"
                                                      "    loop_from_facevert(last_mesh, last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_tri[2]\n",
                                                      primVerts[c%3].uvs[j], primVerts[(c+2)%3].uvs[j], primVerts[(c+1)%3].uvs[j],
                                                      primVerts[c%3].pos, j,
                                                      primVerts[(c+2)%3].pos, j,
                                                      primVerts[(c+1)%3].pos, j);
                                        else
                                            os.format("    loop_from_facevert(last_mesh, last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n"
                                                      "    loop_from_facevert(last_mesh, last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n"
                                                      "    loop_from_facevert(last_mesh, last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n",
                                                      primVerts[c%3].pos, j, primVerts[c%3].uvs[j],
                                                      primVerts[(c+2)%3].pos, j, primVerts[(c+2)%3].uvs[j],
                                                      primVerts[(c+1)%3].pos, j, primVerts[(c+1)%3].uvs[j]);
                                    }
                                }
                            }
                            else
                            {
                                os.format("last_face, last_mesh = add_triangle(bm, bm.verts, (%u,%u,%u), norm_list, (%u,%u,%u), %u, od_list)\n",
                                          primVerts[c%3].pos,
                                          primVerts[(c+1)%3].pos,
                                          primVerts[(c+2)%3].pos,
                                          primVerts[c%3].norm,
                                          primVerts[(c+1)%3].norm,
                                          primVerts[(c+2)%3].norm,
                                          sHead.matIdx);
                                if (matUVCount)
                                {
                                    os << "if last_face is not None:\n";
                                    for (unsigned j=0 ; j<matUVCount ; ++j)
                                    {
                                        if (j==0 && matShortUVs)
                                            os.format("    uv_tri = expand_lightmap_triangle(suv_list[%u], suv_list[%u], suv_list[%u])\n"
                                                      "    loop_from_facevert(last_mesh, last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_tri[0]\n"
                                                      "    loop_from_facevert(last_mesh, last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_tri[1]\n"
                                                      "    loop_from_facevert(last_mesh, last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_tri[2]\n",
                                                      primVerts[c%3].uvs[j], primVerts[(c+1)%3].uvs[j], primVerts[(c+2)%3].uvs[j],
                                                      primVerts[c%3].pos, j,
                                                      primVerts[(c+1)%3].pos, j,
                                                      primVerts[(c+2)%3].pos, j);
                                        else
                                            os.format("    loop_from_facevert(last_mesh, last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n"
                                                      "    loop_from_facevert(last_mesh, last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n"
                                                      "    loop_from_facevert(last_mesh, last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n",
                                                      primVerts[c%3].pos, j, primVerts[c%3].uvs[j],
                                                      primVerts[(c+1)%3].pos, j, primVerts[(c+1)%3].uvs[j],
                                                      primVerts[(c+2)%3].pos, j, primVerts[(c+2)%3].uvs[j]);
                                    }
                                }
                            }
                            flip ^= 1;

                            bool peek = (v >= vertCount - 3);

                            /* Advance one prim vert */
                            primVerts[c%3] = dl.readVert(peek);
                            ++c;
                        }
                    }
                    else if (ptype == GX::TRIANGLES)
                    {
                        for (int v=0 ; v<vertCount ; v+=3)
                        {

                            os.format("last_face, last_mesh = add_triangle(bm, bm.verts, (%u,%u,%u), norm_list, (%u,%u,%u), %u, od_list)\n",
                                      primVerts[0].pos,
                                      primVerts[1].pos,
                                      primVerts[2].pos,
                                      primVerts[0].norm,
                                      primVerts[1].norm,
                                      primVerts[2].norm,
                                      sHead.matIdx);
                            if (matUVCount)
                            {
                                os << "if last_face is not None:\n";
                                for (unsigned j=0 ; j<matUVCount ; ++j)
                                {
                                    if (j==0 && matShortUVs)
                                        os.format("    uv_tri = expand_lightmap_triangle(suv_list[%u], suv_list[%u], suv_list[%u])\n"
                                                  "    loop_from_facevert(last_mesh, last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_tri[0]\n"
                                                  "    loop_from_facevert(last_mesh, last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_tri[1]\n"
                                                  "    loop_from_facevert(last_mesh, last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_tri[2]\n",
                                                  primVerts[0].uvs[j], primVerts[1].uvs[j], primVerts[2].uvs[j],
                                                  primVerts[0].pos, j,
                                                  primVerts[1].pos, j,
                                                  primVerts[2].pos, j);
                                    else
                                        os.format("    loop_from_facevert(last_mesh, last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n"
                                                  "    loop_from_facevert(last_mesh, last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n"
                                                  "    loop_from_facevert(last_mesh, last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n",
                                                  primVerts[0].pos, j, primVerts[0].uvs[j],
                                                  primVerts[1].pos, j, primVerts[1].uvs[j],
                                                  primVerts[2].pos, j, primVerts[2].uvs[j]);
                                }
                            }

                            /* Break if done */
                            if (v+3 >= vertCount)
                                break;

                            /* Advance 3 Prim Verts */
                            for (int pv=0 ; pv<3 ; ++pv)
                                primVerts[pv] = dl.readVert();
                        }
                    }
                    else if (ptype == GX::TRIANGLEFAN)
                    {
                        ++c;
                        for (int v=0 ; v<vertCount-2 ; ++v)
                        {
                            os.format("last_face, last_mesh = add_triangle(bm, bm.verts, (%u,%u,%u), norm_list, (%u,%u,%u), %u, od_list)\n",
                                      firstPrimVert.pos,
                                      primVerts[c%3].pos,
                                      primVerts[(c+1)%3].pos,
                                      firstPrimVert.norm,
                                      primVerts[c%3].norm,
                                      primVerts[(c+1)%3].norm,
                                      sHead.matIdx);
                            if (matUVCount)
                            {
                                os << "if last_face is not None:\n";
                                for (unsigned j=0 ; j<matUVCount ; ++j)
                                {
                                    if (j==0 && matShortUVs)
                                        os.format("    uv_tri = expand_lightmap_triangle(suv_list[%u], suv_list[%u], suv_list[%u])\n"
                                                  "    loop_from_facevert(last_mesh, last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_tri[0]\n"
                                                  "    loop_from_facevert(last_mesh, last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_tri[1]\n"
                                                  "    loop_from_facevert(last_mesh, last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_tri[2]\n",
                                                  firstPrimVert.uvs[j], primVerts[c%3].uvs[j], primVerts[(c+1)%3].uvs[j],
                                                  firstPrimVert.pos, j,
                                                  primVerts[c%3].pos, j,
                                                  primVerts[(c+1)%3].pos, j);
                                    else
                                        os.format("    loop_from_facevert(last_mesh, last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n"
                                                  "    loop_from_facevert(last_mesh, last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n"
                                                  "    loop_from_facevert(last_mesh, last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n",
                                                  firstPrimVert.pos, j, firstPrimVert.uvs[j],
                                                  primVerts[c%3].pos, j, primVerts[c%3].uvs[j],
                                                  primVerts[(c+1)%3].pos, j, primVerts[(c+1)%3].uvs[j]);
                                }
                            }

                            /* Break if done */
                            if (v+3 >= vertCount)
                                break;

                            /* Advance one prim vert */
                            primVerts[(c+2)%3] = dl.readVert();
                            ++c;
                        }
                    }
                    os << "\n";
                }
            }
            }
        }

        if (s < secCount - 1)
            reader.seek(secStart + secSizes[s], athena::Begin);
    }

    /* Finish Mesh */
    FinishBlenderMesh(os, matSetCount, meshIdx);

    if (rp.first)
        rp.second->sendVertexGroupsToBlender(os);

    return lastDlSec;
}

template <class PAKRouter, class MaterialSet, class RigPair, class SurfaceHeader, atUint32 Version>
bool ReadCMDLToBlender(hecl::BlenderConnection& conn,
                       athena::io::IStreamReader& reader,
                       PAKRouter& pakRouter,
                       const typename PAKRouter::EntryType& entry,
                       const SpecBase& dataspec,
                       const RigPair& rp)
{
    Header head;
    head.read(reader);

    if (head.magic != 0xDEADBABE)
    {
        LogDNACommon.report(logvisor::Error, "invalid CMDL magic");
        return false;
    }

    if (head.version != Version)
    {
        LogDNACommon.report(logvisor::Error, "invalid CMDL version");
        return false;
    }

    /* Open Py Stream and read sections */
    hecl::BlenderConnection::PyOutStream os = conn.beginPythonOut(true);
    os.format("import bpy\n"
              "import bmesh\n"
              "\n"
              "bpy.context.scene.name = '%s'\n"
              "bpy.context.scene.hecl_mesh_obj = bpy.context.scene.name\n",
              pakRouter.getBestEntryName(entry).c_str());
    InitGeomBlenderContext(os, dataspec.getMasterShaderPath());
    MaterialSet::RegisterMaterialProps(os);

    os << "# Materials\n"
          "materials = []\n"
          "\n";

    std::vector<VertexAttributes> vertAttribs;
    ReadGeomSectionsToBlender<PAKRouter, MaterialSet, RigPair, SurfaceHeader>
            (os, reader, pakRouter, entry, rp, head.flags.shortNormals(),
             head.flags.shortUVs(), vertAttribs, -1,
             head.secCount, head.matSetCount, head.secSizes.data());

    return true;
}

template bool ReadCMDLToBlender<PAKRouter<DNAMP1::PAKBridge>, DNAMP1::MaterialSet,
                                std::pair<DNAMP1::CSKR*,DNAMP1::CINF*>, DNACMDL::SurfaceHeader_1, 2>
(hecl::BlenderConnection& conn,
 athena::io::IStreamReader& reader,
 PAKRouter<DNAMP1::PAKBridge>& pakRouter,
 const PAKRouter<DNAMP1::PAKBridge>::EntryType& entry,
 const SpecBase& dataspec,
 const std::pair<DNAMP1::CSKR*,DNAMP1::CINF*>& rp);

template bool ReadCMDLToBlender<PAKRouter<DNAMP2::PAKBridge>, DNAMP2::MaterialSet,
                                std::pair<DNAMP2::CSKR*,DNAMP2::CINF*>, DNACMDL::SurfaceHeader_2, 4>
(hecl::BlenderConnection& conn,
 athena::io::IStreamReader& reader,
 PAKRouter<DNAMP2::PAKBridge>& pakRouter,
 const PAKRouter<DNAMP2::PAKBridge>::EntryType& entry,
 const SpecBase& dataspec,
 const std::pair<DNAMP2::CSKR*,DNAMP2::CINF*>& rp);

template bool ReadCMDLToBlender<PAKRouter<DNAMP3::PAKBridge>, DNAMP3::MaterialSet,
                                std::pair<DNAMP3::CSKR*,DNAMP3::CINF*>, DNACMDL::SurfaceHeader_3, 4>
(hecl::BlenderConnection& conn,
 athena::io::IStreamReader& reader,
 PAKRouter<DNAMP3::PAKBridge>& pakRouter,
 const PAKRouter<DNAMP3::PAKBridge>::EntryType& entry,
 const SpecBase& dataspec,
 const std::pair<DNAMP3::CSKR*,DNAMP3::CINF*>& rp);

template bool ReadCMDLToBlender<PAKRouter<DNAMP3::PAKBridge>, DNAMP3::MaterialSet,
                                std::pair<DNAMP3::CSKR*,DNAMP3::CINF*>, DNACMDL::SurfaceHeader_3, 5>
(hecl::BlenderConnection& conn,
 athena::io::IStreamReader& reader,
 PAKRouter<DNAMP3::PAKBridge>& pakRouter,
 const PAKRouter<DNAMP3::PAKBridge>::EntryType& entry,
 const SpecBase& dataspec,
 const std::pair<DNAMP3::CSKR*,DNAMP3::CINF*>& rp);

template <class PAKRouter, class MaterialSet>
void NameCMDL(athena::io::IStreamReader& reader,
              PAKRouter& pakRouter,
              typename PAKRouter::EntryType& entry,
              const SpecBase& dataspec)
{
    Header head;
    head.read(reader);
    std::string bestName = hecl::Format("CMDL_%s", entry.id.toString().c_str());

    /* Pre-read pass to determine maximum used vert indices */
    atUint32 matSecCount = 0;
    if (head.matSetCount)
        matSecCount = MaterialSet::OneSection() ? 1 : head.matSetCount;
    atUint32 lastDlSec = head.secCount;
    for (size_t s=0 ; s<lastDlSec ; ++s)
    {
        atUint64 secStart = reader.position();
        if (s < matSecCount)
        {
            MaterialSet matSet;
            matSet.read(reader);
            matSet.nameTextures(pakRouter, bestName.c_str(), s);
        }

        if (s < head.secCount - 1)
            reader.seek(secStart + head.secSizes[s], athena::Begin);
    }
}

template void NameCMDL<PAKRouter<DNAMP1::PAKBridge>, DNAMP1::MaterialSet>
(athena::io::IStreamReader& reader,
 PAKRouter<DNAMP1::PAKBridge>& pakRouter,
 PAKRouter<DNAMP1::PAKBridge>::EntryType& entry,
 const SpecBase& dataspec);

static void WriteDLVal(athena::io::FileWriter& writer, GX::AttrType type, atUint32 val)
{
    switch (type)
    {
    case GX::DIRECT:
    case GX::INDEX8:
        writer.writeUByte(atUint8(val));
        break;
    case GX::INDEX16:
        writer.writeUint16Big(atUint16(val));
        break;
    default: break;
    }
}

template <class MaterialSet, class SurfaceHeader, atUint32 Version>
bool WriteCMDL(const hecl::ProjectPath& outPath, const hecl::ProjectPath& inPath, const Mesh& mesh)
{
    Header head;
    head.magic = 0xDEADBABE;
    head.version = Version;
    head.aabbMin = mesh.aabbMin.val;
    head.aabbMax = mesh.aabbMax.val;
    head.matSetCount = mesh.materialSets.size();
    head.secCount = head.matSetCount + 5 + mesh.surfaces.size();
    head.secSizes.reserve(head.secCount);

    /* Lengths of padding to insert while writing */
    std::vector<size_t> paddingSizes;
    paddingSizes.reserve(head.secCount);

    /* Build material sets */
    std::vector<MaterialSet> matSets;
    matSets.reserve(mesh.materialSets.size());
    {
        hecl::Frontend::Frontend FE;
        for (const std::vector<Mesh::Material>& mset : mesh.materialSets)
        {
            matSets.emplace_back();
            MaterialSet& targetMSet = matSets.back();
            std::vector<hecl::ProjectPath> texPaths;
            std::vector<hecl::Backend::GX> setBackends;
            setBackends.reserve(mset.size());

            size_t endOff = 0;
            atUint32 nextGroupIdx = 0;
            for (const Mesh::Material& mat : mset)
            {
                std::string diagName = hecl::Format("%s:%s", inPath.getLastComponentUTF8(), mat.name.c_str());
                hecl::Frontend::IR matIR = FE.compileSource(mat.source, diagName);
                setBackends.emplace_back();
                hecl::Backend::GX& matGX = setBackends.back();
                matGX.reset(matIR, FE.getDiagnostics());

                atUint32 groupIdx = -1;
                if (matSets.size() == 1)
                {
                    for (size_t i=0 ; i<setBackends.size()-1 ; ++i)
                    {
                        const hecl::Backend::GX& other = setBackends[i];
                        if (matGX == other)
                        {
                            groupIdx = i;
                            break;
                        }
                    }
                    if (groupIdx == -1)
                        groupIdx = nextGroupIdx++;
                }

                targetMSet.materials.emplace_back(matGX, mat.iprops, mat.texs, texPaths,
                                                  mesh.colorLayerCount, mesh.uvLayerCount,
                                                  false, false, groupIdx);

                endOff = targetMSet.materials.back().binarySize(endOff);
                targetMSet.head.addMaterialEndOff(endOff);
            }

            for (const hecl::ProjectPath& path : texPaths)
            {
                const hecl::SystemString& relPath = path.getRelativePath();

                /* TODO: incorporate hecl hashes */
                size_t search = relPath.find(_S("TXTR_"));
                if (search != hecl::SystemString::npos)
                    targetMSet.head.addTexture(relPath.c_str() + search + 5);
                else
                    LogDNACommon.report(logvisor::Fatal, "unable to get hash from path");
            }

            size_t secSz = targetMSet.binarySize(0);
            size_t secSz32 = ROUND_UP_32(secSz);
            head.secSizes.push_back(secSz32);
            paddingSizes.push_back(secSz32 - secSz);
        }
    }

    /* Vertex Positions */
    size_t secSz = mesh.pos.size() * 12;
    size_t secSz32 = ROUND_UP_32(secSz);
    if (secSz32 == 0)
        secSz32 = 32;
    head.secSizes.push_back(secSz32);
    paddingSizes.push_back(secSz32 - secSz);

    /* Vertex Normals */
    secSz = mesh.norm.size() * 12;
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
    for (const Mesh::Surface& surf : mesh.surfaces)
    {
        size_t vertSz = matSets.at(0).materials.at(surf.materialIdx).getVAFlags().vertDLSize();
        if (surf.verts.size() > 65536)
            LogDNACommon.report(logvisor::Fatal, "GX DisplayList overflow");
        size_t secSz = 67 + surf.verts.size() * vertSz;
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
    for (const MaterialSet& mset : matSets)
    {
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
    for (const atVec3f& norm : mesh.norm)
        writer.writeVec3fBig(norm);
    writer.fill(atUint8(0), *padIt);
    ++padIt;

    /* Vertex Colors */
    for (const atVec3f& col : mesh.color)
    {
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

    /* Surface index */
    writer.writeUint32Big(surfEndOffs.size());
    for (size_t off : surfEndOffs)
        writer.writeUint32Big(off);
    writer.fill(atUint8(0), *padIt);
    ++padIt;

    /* Surfaces */
    GX::Primitive prim;
    if (mesh.topology == hecl::HMDLTopology::Triangles)
        prim = GX::TRIANGLES;
    else if (mesh.topology == hecl::HMDLTopology::TriStrips)
        prim = GX::TRIANGLESTRIP;
    else
        LogDNACommon.report(logvisor::Fatal, "unrecognized mesh output mode");
    for (const Mesh::Surface& surf : mesh.surfaces)
    {
        const typename MaterialSet::Material::VAFlags& vaFlags =
        matSets.at(0).materials.at(surf.materialIdx).getVAFlags();
        size_t vertSz = vaFlags.vertDLSize();

        SurfaceHeader header;
        header.centroid = surf.centroid;
        header.matIdx = surf.materialIdx;
        header.dlSize = 3 + surf.verts.size() * vertSz;
        header.reflectionNormal = surf.reflectionNormal;
        header.write(writer);

        writer.writeUByte(prim);
        writer.writeUint16Big(surf.verts.size());

        for (const Mesh::Surface::Vert& vert : surf.verts)
        {
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

        writer.fill(atUint8(0), *padIt);
        ++padIt;
    }

    writer.close();
    return true;
}

template bool WriteCMDL<DNAMP1::MaterialSet, DNACMDL::SurfaceHeader_1, 2>
(const hecl::ProjectPath& outPath, const hecl::ProjectPath& inPath, const Mesh& mesh);

template <class MaterialSet, class SurfaceHeader, atUint32 Version>
bool WriteHMDLCMDL(const hecl::ProjectPath& outPath, const hecl::ProjectPath& inPath, const Mesh& mesh)
{
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
    {
        hecl::Frontend::Frontend FE;
        for (const std::vector<Mesh::Material>& mset : mesh.materialSets)
        {
            matSets.emplace_back();
            MaterialSet& targetMSet = matSets.back();
            std::vector<hecl::ProjectPath> texPaths;
            texPaths.reserve(mset.size()*4);
            for (const Mesh::Material& mat : mset)
            {
                for (const hecl::ProjectPath& path : mat.texs)
                {
                    bool found = false;
                    for (const hecl::ProjectPath& ePath : texPaths)
                    {
                        if (path == ePath)
                        {
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                        texPaths.push_back(path);
                }
            }

            size_t endOff = 0;
            for (const Mesh::Material& mat : mset)
            {
                std::string diagName = hecl::Format("%s:%s", inPath.getLastComponentUTF8(), mat.name.c_str());
                targetMSet.materials.emplace_back(FE, diagName, mat, mat.iprops, texPaths);
                endOff = targetMSet.materials.back().binarySize(endOff);
                targetMSet.head.addMaterialEndOff(endOff);
            }

            for (const hecl::ProjectPath& path : texPaths)
            {
                const hecl::SystemString& relPath = path.getRelativePath();

                /* TODO: incorporate hecl hashes */
                size_t search = relPath.find(_S("TXTR_"));
                if (search != hecl::SystemString::npos)
                    targetMSet.head.addTexture(relPath.c_str() + search + 5);
                else
                    LogDNACommon.report(logvisor::Fatal, "unable to get hash from path");
            }

            size_t secSz = targetMSet.binarySize(0);
            size_t secSz32 = ROUND_UP_32(secSz);
            head.secSizes.push_back(secSz32);
            paddingSizes.push_back(secSz32 - secSz);
        }
    }

    hecl::HMDLBuffers bufs = mesh.getHMDLBuffers();

    /* Metadata */
    size_t secSz = bufs.m_meta.binarySize(0);
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
    for (const hecl::HMDLBuffers::Surface& surf : bufs.m_surfaces)
    {
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
    for (const MaterialSet& mset : matSets)
    {
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
    for (const hecl::HMDLBuffers::Surface& surf : bufs.m_surfaces)
    {
        const Mesh::Surface& osurf = surf.m_origSurf;

        SurfaceHeader header;
        header.centroid = osurf.centroid;
        header.matIdx = osurf.materialIdx;
        header.reflectionNormal = osurf.reflectionNormal;
        header.unk1 = surf.m_start;
        header.unk2 = surf.m_count;
        header.write(writer);

        writer.fill(atUint8(0), *padIt);
        ++padIt;
    }

    /* Ensure final surface's alignment writes zeros */
    writer.seek(-1, athena::Current);
    writer.writeUByte(0);
    writer.close();
    return true;
}

}
}
