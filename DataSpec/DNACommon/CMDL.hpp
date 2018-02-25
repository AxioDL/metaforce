#ifndef _DNACOMMON_CMDL_HPP_
#define _DNACOMMON_CMDL_HPP_

#include "athena/FileWriter.hpp"
#include "hecl/Frontend.hpp"
#include "hecl/Backend/GX.hpp"
#include "PAK.hpp"
#include "GX.hpp"
#include "TXTR.hpp"
#include "zeus/CAABox.hpp"

namespace DataSpec::DNACMDL
{

using Mesh = hecl::blender::Mesh;
using Material = hecl::blender::Material;

struct Header : BigDNA
{
    AT_DECL_DNA
    Value<atUint32> magic;
    Value<atUint32> version;
    struct Flags : BigDNA
    {
        AT_DECL_DNA
        Value<atUint32> flags = 0;
        bool shortNormals() const {return (flags & 0x2) != 0;}
        void setShortNormals(bool val) {flags &= ~0x2; flags |= val << 1;}
        bool shortUVs() const {return (flags & 0x4) != 0;}
        void setShortUVs(bool val) {flags &= ~0x4; flags |= val << 2;}
    } flags;
    Value<atVec3f> aabbMin;
    Value<atVec3f> aabbMax;
    Value<atUint32> secCount;
    Value<atUint32> matSetCount;
    Vector<atUint32, AT_DNA_COUNT(secCount)> secSizes;
    Align<32> align;
};

struct SurfaceHeader_1 : BigDNA
{
    AT_DECL_EXPLICIT_DNA
    Value<atVec3f> centroid;
    Value<atUint32> matIdx = 0;
    Value<atUint16> qDiv = 0x8000;
    Value<atUint16> dlSize = 0;
    Value<atUint32> idxStart = 0; /* Actually used by game to stash CCubeModel pointer */
    Value<atUint32> idxCount = 0; /* Actually used by game to stash next CCubeSurface pointer */
    Value<atUint32> aabbSz = 0;
    Value<atVec3f> reflectionNormal;
    Value<atVec3f> aabb[2];
    Align<32> align;

    static constexpr bool UseMatrixSkinning() {return false;}
    static constexpr atInt16 skinMatrixBankIdx() {return -1;}
};

struct SurfaceHeader_2 : BigDNA
{
    AT_DECL_EXPLICIT_DNA
    Value<atVec3f> centroid;
    Value<atUint32> matIdx = 0;
    Value<atUint16> qDiv = 0x8000;
    Value<atUint16> dlSize = 0;
    Value<atUint32> idxStart = 0; /* Actually used by game to stash CCubeModel pointer */
    Value<atUint32> idxCount = 0; /* Actually used by game to stash next CCubeSurface pointer */
    Value<atUint32> aabbSz = 0;
    Value<atVec3f> reflectionNormal;
    Value<atInt16> skinMtxBankIdx;
    Value<atUint16> surfaceGroup;
    Value<atVec3f> aabb[2];
    Align<32> align;

    static constexpr bool UseMatrixSkinning() {return false;}
    atInt16 skinMatrixBankIdx() const {return skinMtxBankIdx;}
};

struct SurfaceHeader_3 : BigDNA
{
    AT_DECL_EXPLICIT_DNA
    Value<atVec3f> centroid;
    Value<atUint32> matIdx = 0;
    Value<atUint16> qDiv = 0x8000;
    Value<atUint16> dlSize = 0;
    Value<atUint32> idxStart = 0; /* Actually used by game to stash CCubeModel pointer */
    Value<atUint32> idxCount = 0; /* Actually used by game to stash next CCubeSurface pointer */
    Value<atUint32> aabbSz = 0;
    Value<atVec3f> reflectionNormal;
    Value<atInt16> skinMtxBankIdx;
    Value<atUint16> surfaceGroup;
    Value<atVec3f> aabb[2];
    Value<atUint8> unk3;
    Align<32> align;

    static constexpr bool UseMatrixSkinning() {return true;}
    atInt16 skinMatrixBankIdx() const {return skinMtxBankIdx;}
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
                         std::vector<VertexAttributes>& attributesOut);

template <class PAKRouter, class MaterialSet>
void ReadMaterialSetToBlender_1_2(hecl::blender::PyOutStream& os,
                                  const MaterialSet& matSet,
                                  const PAKRouter& pakRouter,
                                  const typename PAKRouter::EntryType& entry,
                                  unsigned setIdx);

template <class PAKRouter, class MaterialSet>
void ReadMaterialSetToBlender_3(hecl::blender::PyOutStream& os,
                                const MaterialSet& matSet,
                                const PAKRouter& pakRouter,
                                const typename PAKRouter::EntryType& entry,
                                unsigned setIdx);

void InitGeomBlenderContext(hecl::blender::PyOutStream& os,
                            const hecl::ProjectPath& masterShaderPath,
                            bool solidShading);
void FinishBlenderMesh(hecl::blender::PyOutStream& os,
                       unsigned matSetCount, int meshIdx);

template <class PAKRouter, class MaterialSet, class RigPair, class SurfaceHeader>
atUint32 ReadGeomSectionsToBlender(hecl::blender::PyOutStream& os,
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
                                   atUint32 surfaceCount=0);

template <class PAKRouter, class MaterialSet, class RigPair, class SurfaceHeader, atUint32 Version>
bool ReadCMDLToBlender(hecl::blender::Connection& conn,
                       athena::io::IStreamReader& reader,
                       PAKRouter& pakRouter,
                       const typename PAKRouter::EntryType& entry,
                       const SpecBase& dataspec,
                       const RigPair& rp);

template <class PAKRouter, class MaterialSet>
void NameCMDL(athena::io::IStreamReader& reader,
              PAKRouter& pakRouter,
              typename PAKRouter::EntryType& entry,
              const SpecBase& dataspec);

template <class MaterialSet, class SurfaceHeader, atUint32 Version>
bool WriteCMDL(const hecl::ProjectPath& outPath, const hecl::ProjectPath& inPath, const Mesh& mesh);

template <class MaterialSet, class SurfaceHeader, atUint32 Version>
bool WriteHMDLCMDL(const hecl::ProjectPath& outPath, const hecl::ProjectPath& inPath,
                   const Mesh& mesh, hecl::blender::PoolSkinIndex& poolSkinIndex);

template <class MaterialSet, class SurfaceHeader, class MeshHeader>
bool WriteMREASecs(std::vector<std::vector<uint8_t>>& secsOut, const hecl::ProjectPath& inPath,
                   const std::vector<Mesh>& meshes, zeus::CAABox& fullAABB, std::vector<zeus::CAABox>& meshAABBs);

template <class MaterialSet, class SurfaceHeader, class MeshHeader>
bool WriteHMDLMREASecs(std::vector<std::vector<uint8_t>>& secsOut, const hecl::ProjectPath& inPath,
                       const std::vector<Mesh>& meshes, zeus::CAABox& fullAABB, std::vector<zeus::CAABox>& meshAABBs);

}

#endif // _DNACOMMON_CMDL_HPP_
