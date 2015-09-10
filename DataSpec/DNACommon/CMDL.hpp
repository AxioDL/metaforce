#ifndef _DNACOMMON_CMDL_HPP_
#define _DNACOMMON_CMDL_HPP_

#include "PAK.hpp"
#include "BlenderConnection.hpp"
#include "GX.hpp"
#include "TXTR.hpp"

namespace Retro
{
namespace DNACMDL
{

struct Header : BigDNA
{
    DECL_DNA
    Value<atUint32> magic;
    Value<atUint32> version;
    struct Flags : BigDNA
    {
        DECL_DNA
        Value<atUint32> flags;
        inline bool shortNormals() const {return (flags & 0x2) != 0;}
        inline void setShortNormals(bool val) {flags &= ~0x2; flags |= val << 1;}
        inline bool shortUVs() const {return (flags & 0x4) != 0;}
        inline void setShortUVs(bool val) {flags &= ~0x4; flags |= val << 2;}
    } flags;
    Value<atVec3f> aabbMin;
    Value<atVec3f> aabbMax;
    Value<atUint32> secCount;
    Value<atUint32> matSetCount;
    Vector<atUint32, DNA_COUNT(secCount)> secSizes;
    Align<32> align;
};

struct SurfaceHeader : BigDNA
{
    DECL_DNA
    Value<atVec3f> centroid;
    Value<atUint32> matIdx;
    Value<atInt16> qDiv;
    Value<atUint16> dlSize;
    Seek<8, Athena::Current> seek;
    Value<atUint32> aabbSz;
    Value<atVec3f> reflectionNormal;
    Seek<DNA_COUNT(aabbSz), Athena::Current> seek2;
    Align<32> align;
};

struct VertexAttributes
{
    GX::AttrType pos = GX::NONE;
    GX::AttrType norm = GX::NONE;
    GX::AttrType color0 = GX::NONE;
    GX::AttrType color1 = GX::NONE;
    unsigned uvCount = 0;
    GX::AttrType uvs[7] = {GX::NONE};
    GX::AttrType pnMtxIdx = GX::NONE;
    unsigned texMtxIdxCount = 0;
    GX::AttrType texMtxIdx[7] = {GX::NONE};
    bool shortUVs;
};

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
void ReadMaterialSetToBlender_1_2(HECL::BlenderConnection::PyOutStream& os,
                                  const MaterialSet& matSet,
                                  const PAKRouter& pakRouter,
                                  const typename PAKRouter::EntryType& entry,
                                  unsigned setIdx,
                                  const SpecBase& dataspec)
{
    /* Texmaps */
    os << "texmap_list = []\n";
    for (const UniqueID32& tex : matSet.head.textureIDs)
    {
        std::string texName = pakRouter.getBestEntryName(tex);
        const NOD::DiscBase::IPartition::Node* node;
        const typename PAKRouter::EntryType* texEntry = pakRouter.lookupEntry(tex, &node);
        HECL::ProjectPath txtrPath = pakRouter.getWorking(texEntry);
        if (txtrPath.getPathType() == HECL::ProjectPath::PT_NONE)
        {
            PAKEntryReadStream rs = texEntry->beginReadStream(*node);
            TXTR::Extract(dataspec, rs, txtrPath);
        }
        HECL::SystemString resPath = pakRouter.getResourceRelativePath(entry, tex);
        HECL::SystemUTF8View resPathView(resPath);
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
void ReadMaterialSetToBlender_3(HECL::BlenderConnection::PyOutStream& os,
                                const MaterialSet& matSet,
                                const PAKRouter& pakRouter,
                                const typename PAKRouter::EntryType& entry,
                                unsigned setIdx,
                                const SpecBase& dataspec)
{
    unsigned m=0;
    for (const typename MaterialSet::Material& mat : matSet.materials)
    {
        MaterialSet::ConstructMaterial(os, mat, setIdx, m++);
        os << "materials.append(new_material)\n";
    }
}

class DLReader
{
    const VertexAttributes& m_va;
    std::unique_ptr<atUint8[]> m_dl;
    size_t m_dlSize;
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
            retval = HECL::SBig(*(atUint16*)m_cur);
            m_cur += 2;
            break;
        default: break;
        }
        return retval;
    }
public:
    DLReader(const VertexAttributes& va, std::unique_ptr<atUint8[]>&& dl, size_t dlSize)
    : m_va(va), m_dl(std::move(dl)), m_dlSize(dlSize)
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
    atUint16 readVertCount()
    {
        atUint16 retval = HECL::SBig(*(atUint16*)m_cur);
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
                out.pnMtxIdx = MAX(out.pnMtxIdx, val);
                val = readVal(m_va.texMtxIdx[0]);
                out.texMtxIdx[0] = MAX(out.texMtxIdx[0], val);
                val = readVal(m_va.texMtxIdx[1]);
                out.texMtxIdx[1] = MAX(out.texMtxIdx[1], val);
                val = readVal(m_va.texMtxIdx[2]);
                out.texMtxIdx[2] = MAX(out.texMtxIdx[2], val);
                val = readVal(m_va.texMtxIdx[3]);
                out.texMtxIdx[3] = MAX(out.texMtxIdx[3], val);
                val = readVal(m_va.texMtxIdx[4]);
                out.texMtxIdx[4] = MAX(out.texMtxIdx[4], val);
                val = readVal(m_va.texMtxIdx[5]);
                out.texMtxIdx[5] = MAX(out.texMtxIdx[5], val);
                val = readVal(m_va.texMtxIdx[6]);
                out.texMtxIdx[6] = MAX(out.texMtxIdx[6], val);
                val = readVal(m_va.pos);
                out.pos = MAX(out.pos, val);
                val = readVal(m_va.norm);
                out.norm = MAX(out.norm, val);
                val = readVal(m_va.color0);
                out.color[0] = MAX(out.color[0], val);
                val = readVal(m_va.color1);
                out.color[1] = MAX(out.color[1], val);
                val = readVal(m_va.uvs[0]);
                out.uvs[0] = MAX(out.uvs[0], val);
                val = readVal(m_va.uvs[1]);
                out.uvs[1] = MAX(out.uvs[1], val);
                val = readVal(m_va.uvs[2]);
                out.uvs[2] = MAX(out.uvs[2], val);
                val = readVal(m_va.uvs[3]);
                out.uvs[3] = MAX(out.uvs[3], val);
                val = readVal(m_va.uvs[4]);
                out.uvs[4] = MAX(out.uvs[4], val);
                val = readVal(m_va.uvs[5]);
                out.uvs[5] = MAX(out.uvs[5], val);
                val = readVal(m_va.uvs[6]);
                out.uvs[6] = MAX(out.uvs[6], val);
            }
        }
        m_cur = bakCur;
    }
};

void InitGeomBlenderContext(HECL::BlenderConnection::PyOutStream& os,
                            const HECL::ProjectPath& masterShaderPath);
void FinishBlenderMesh(HECL::BlenderConnection::PyOutStream& os,
                       unsigned matSetCount, int meshIdx);

template <class PAKRouter, class MaterialSet, class RIGPAIR>
atUint32 ReadGeomSectionsToBlender(HECL::BlenderConnection::PyOutStream& os,
                                   Athena::io::IStreamReader& reader,
                                   PAKRouter& pakRouter,
                                   const typename PAKRouter::EntryType& entry,
                                   const SpecBase& dataspec,
                                   const RIGPAIR& rp,
                                   bool shortNormals,
                                   bool shortUVs,
                                   std::vector<VertexAttributes>& vertAttribs,
                                   int meshIdx,
                                   atUint32 secCount,
                                   atUint32 matSetCount,
                                   const atUint32* secSizes)
{
    os << "# Begin bmesh\n"
          "bm = bmesh.new()\n"
          "\n";

    if (rp.first)
        os << "dvert_lay = bm.verts.layers.deform.verify()\n";

    os << "# Overdraw-tracking\n"
          "od_list = []\n"
          "\n";

    /* Pre-read pass to determine maximum used vert indices */
    bool visitedDLOffsets = false;
    atUint32 lastDlSec = secCount;
    atUint64 afterHeaderPos = reader.position();
    DLReader::DLPrimVert maxIdxs;
    for (size_t s=0 ; s<lastDlSec ; ++s)
    {
        atUint64 secStart = reader.position();
        if (s < matSetCount)
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
            switch (s-matSetCount)
            {
            case 0:
            {
                /* Positions */
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
                /* Short UVs */
                if (shortUVs)
                    break;

                /* DL Offsets (here or next section) */
                visitedDLOffsets = true;
                lastDlSec = s + reader.readUint32Big() + 1;
                break;
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

                /* Do max index pre-read */
                atUint32 realDlSize = secSizes[s] - (reader.position() - secStart);
                DLReader dl(vertAttribs[sHead.matIdx], reader.readUBytes(realDlSize), realDlSize);
                dl.preReadMaxIdxs(maxIdxs);

            }
            }
        }

        if (s < secCount - 1)
            reader.seek(secStart + secSizes[s], Athena::Begin);
    }

    reader.seek(afterHeaderPos, Athena::Begin);

    visitedDLOffsets = false;
    unsigned createdUVLayers = 0;
    unsigned surfIdx = 0;

    for (size_t s=0 ; s<lastDlSec ; ++s)
    {
        atUint64 secStart = reader.position();
        if (s < matSetCount)
        {
            MaterialSet matSet;
            matSet.read(reader);
            matSet.readToBlender(os, pakRouter, entry, s, dataspec);
            if (!s)
                GetVertexAttributes(matSet, vertAttribs);
        }
        else
        {
            switch (s-matSetCount)
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
                        rp.first->weightVertex(os, *rp.second, i);
                }
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

                os.format("materials[%u].pass_index = %u\n", sHead.matIdx, surfIdx++);
                if (matUVCount > createdUVLayers)
                {
                    for (unsigned l=createdUVLayers ; l<matUVCount ; ++l)
                        os.format("bm.loops.layers.uv.new('UV_%u')\n", l);
                    createdUVLayers = matUVCount;
                }

                atUint32 realDlSize = secSizes[s] - (reader.position() - secStart);
                DLReader dl(vertAttribs[sHead.matIdx], reader.readUBytes(realDlSize), realDlSize);

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
                                            os.format("    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = suv_list[%u]\n"
                                                      "    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = suv_list[%u]\n"
                                                      "    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = suv_list[%u]\n",
                                                      primVerts[c%3].pos, j, primVerts[c%3].uvs[j],
                                                      primVerts[(c+2)%3].pos, j, primVerts[(c+2)%3].uvs[j],
                                                      primVerts[(c+1)%3].pos, j, primVerts[(c+1)%3].uvs[j]);
                                        else
                                            os.format("    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n"
                                                      "    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n"
                                                      "    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n",
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
                                            os.format("    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = suv_list[%u]\n"
                                                      "    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = suv_list[%u]\n"
                                                      "    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = suv_list[%u]\n",
                                                      primVerts[c%3].pos, j, primVerts[c%3].uvs[j],
                                                      primVerts[(c+1)%3].pos, j, primVerts[(c+1)%3].uvs[j],
                                                      primVerts[(c+2)%3].pos, j, primVerts[(c+2)%3].uvs[j]);
                                        else
                                            os.format("    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n"
                                                      "    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n"
                                                      "    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n",
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
                                        os.format("    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = suv_list[%u]\n"
                                                  "    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = suv_list[%u]\n"
                                                  "    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = suv_list[%u]\n",
                                                  primVerts[0].pos, j, primVerts[0].uvs[j],
                                                  primVerts[1].pos, j, primVerts[1].uvs[j],
                                                  primVerts[2].pos, j, primVerts[2].uvs[j]);
                                    else
                                        os.format("    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n"
                                                  "    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n"
                                                  "    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n",
                                                  primVerts[0].pos, j, primVerts[0].uvs[j],
                                                  primVerts[1].pos, j, primVerts[1].uvs[j],
                                                  primVerts[2].pos, j, primVerts[2].uvs[j]);
                                }
                            }

                            /* Break if done */
                            if (v+3 >= vertCount)
                                break;

                            bool peek = (v >= vertCount - 3);

                            /* Advance 3 Prim Verts */
                            for (int pv=0 ; pv<3 ; ++pv)
                                primVerts[pv] = dl.readVert(peek);
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
                                        os.format("    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = suv_list[%u]\n"
                                                  "    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = suv_list[%u]\n"
                                                  "    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = suv_list[%u]\n",
                                                  firstPrimVert.pos, j, firstPrimVert.uvs[j],
                                                  primVerts[c%3].pos, j, primVerts[c%3].uvs[j],
                                                  primVerts[(c+1)%3].pos, j, primVerts[(c+1)%3].uvs[j]);
                                    else
                                        os.format("    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n"
                                                  "    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n"
                                                  "    loop_from_facevert(last_face, %u)[last_mesh.loops.layers.uv[%u]].uv = uv_list[%u]\n",
                                                  firstPrimVert.pos, j, firstPrimVert.uvs[j],
                                                  primVerts[c%3].pos, j, primVerts[c%3].uvs[j],
                                                  primVerts[(c+1)%3].pos, j, primVerts[(c+1)%3].uvs[j]);
                                }
                            }

                            bool peek = (v >= vertCount - 3);

                            /* Advance one prim vert */
                            primVerts[(c+2)%3] = dl.readVert(peek);
                            ++c;
                        }
                    }
                    os << "\n";
                }
            }
            }
        }

        if (s < secCount - 1)
            reader.seek(secStart + secSizes[s], Athena::Begin);
    }

    /* Finish Mesh */
    FinishBlenderMesh(os, matSetCount, meshIdx);

    if (rp.first)
        rp.second->sendVertexGroupsToBlender(os);

    return lastDlSec;
}

template <class PAKRouter, class MaterialSet, class RIGPAIR, atUint32 Version>
bool ReadCMDLToBlender(HECL::BlenderConnection& conn,
                       Athena::io::IStreamReader& reader,
                       PAKRouter& pakRouter,
                       const typename PAKRouter::EntryType& entry,
                       const SpecBase& dataspec,
                       const RIGPAIR& rp)
{
    Header head;
    head.read(reader);

    if (head.magic != 0xDEADBABE)
    {
        LogDNACommon.report(LogVisor::Error, "invalid CMDL magic");
        return false;
    }

    if (head.version != Version)
    {
        LogDNACommon.report(LogVisor::Error, "invalid CMDL version");
        return false;
    }

    /* Open Py Stream and read sections */
    HECL::BlenderConnection::PyOutStream os = conn.beginPythonOut(true);
    os.format("import bpy\n"
              "import bmesh\n"
              "\n"
              "bpy.context.scene.name = '%s'\n"
              "bpy.context.scene.hecl_type = 'MESH'\n"
              "bpy.context.scene.hecl_mesh_obj = bpy.context.scene.name\n",
              pakRouter.getBestEntryName(entry).c_str());
    InitGeomBlenderContext(os, dataspec.getMasterShaderPath());
    MaterialSet::RegisterMaterialProps(os);

    os << "# Materials\n"
          "materials = []\n"
          "\n";

    std::vector<VertexAttributes> vertAttribs;
    ReadGeomSectionsToBlender<PAKRouter, MaterialSet, RIGPAIR>
            (os, reader, pakRouter, entry, dataspec, rp, head.flags.shortNormals(),
             head.flags.shortUVs(), vertAttribs, -1,
             head.secCount, head.matSetCount, head.secSizes.data());

    return true;
}

}
}

#endif // _DNACOMMON_CMDL_HPP_
