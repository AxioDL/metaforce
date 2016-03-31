#include "Graphics/CModel.hpp"
#include "Graphics/CTexture.hpp"
#include "Graphics/CGraphics.hpp"
#include "hecl/HMDLMeta.hpp"
#include "hecl/Runtime.hpp"

namespace urde
{
static logvisor::Module Log("urde::CModelBoo");
bool CBooModel::g_DrawingOccluders = false;

struct SUnskinnedUniforms
{
    zeus::CMatrix4f mv;
    zeus::CMatrix4f mvinv;
    zeus::CMatrix4f proj;
    zeus::CMatrix4f tex[8];
};

CBooModel::CBooModel(std::vector<CBooSurface>* surfaces, SShader& shader,
                     boo::IVertexFormat* vtxFmt, boo::IGraphicsBufferS* vbo, boo::IGraphicsBufferS* ibo,
                     const zeus::CAABox& aabb, u8 shortNormals, bool texturesLoaded)
: x0_surfaces(surfaces), x4_matSet(&shader.m_matSet), m_pipelines(&shader.m_shaders),
  m_vtxFmt(vtxFmt), x8_vbo(vbo), xc_ibo(ibo), x1c_textures(&shader.x0_textures), x20_aabb(aabb),
  x40_24_texturesLoaded(texturesLoaded), x40_25_(0), x41_shortNormals(shortNormals)
{
    for (CBooSurface& surf : *x0_surfaces)
        surf.m_parent = this;

    for (auto it=x0_surfaces->rbegin() ; it != x0_surfaces->rend() ; ++it)
    {
        u32 matId = it->m_data.matIdx;
        const DataSpec::DNAMP1::HMDLMaterialSet::Material& matData = GetMaterialByIndex(matId);
        if (matData.flags.depthSorting())
        {
            it->m_next = x3c_firstSortedSurface;
            x3c_firstSortedSurface = &*it;
        }
        else
        {
            it->m_next = x38_firstUnsortedSurface;
            x38_firstUnsortedSurface = &*it;
        }
    }

    if (x40_24_texturesLoaded)
        BuildGfxToken();
}

void CBooModel::BuildGfxToken()
{
    m_gfxToken = CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        m_uniformBuffer = ctx.newDynamicBuffer(boo::BufferUse::Vertex, sizeof(SUnskinnedUniforms), 1);
        boo::IGraphicsBuffer* bufs[] = {m_uniformBuffer};
        m_shaderDataBindings.reserve(x4_matSet->materials.size());
        auto pipelineIt = m_pipelines->begin();
        std::vector<boo::ITexture*> texs;
        for (const DataSpec::DNAMP1::HMDLMaterialSet::Material& mat : x4_matSet->materials)
        {
            texs.clear();
            texs.reserve(mat.textureIdxs.size());
            for (atUint32 idx : mat.textureIdxs)
            {
                TCachedToken<CTexture>& tex = (*x1c_textures)[idx];
                texs.push_back(tex.GetObj()->GetBooTexture());
            }
            m_shaderDataBindings.push_back(ctx.newShaderDataBinding(*pipelineIt, m_vtxFmt, x8_vbo, nullptr, xc_ibo, 1, bufs,
                                                                    mat.textureIdxs.size(), texs.data()));
            ++pipelineIt;
        }
        return true;
    });
}

void CBooModel::MakeTexuresFromMats(const DataSpec::DNAMP1::HMDLMaterialSet& matSet,
                                    std::vector<TCachedToken<CTexture>>& toksOut,
                                    IObjectStore& store)
{
    toksOut.reserve(matSet.head.textureIDs.size());
    for (const DataSpec::UniqueID32& id : matSet.head.textureIDs)
        toksOut.emplace_back(store.GetObj({SBIG('TXTR'), id.toUint32()}));
}

void CBooModel::RemapMaterialData(SShader& shader)
{
    x4_matSet = &shader.m_matSet;
    x1c_textures = &shader.x0_textures;
    m_pipelines = &shader.m_shaders;
    x40_24_texturesLoaded = false;
    m_gfxToken.doDestroy();
}

bool CBooModel::TryLockTextures() const
{
    if (!x40_24_texturesLoaded)
    {
        bool allLoad = true;
        for (TCachedToken<CTexture>& tex : *x1c_textures)
        {
            tex.Lock();
            if (!tex.IsLoaded())
                allLoad = false;
        }

        if (allLoad)
            ((CBooModel*)this)->BuildGfxToken();

        ((CBooModel*)this)->x40_24_texturesLoaded = allLoad;
    }
    return x40_24_texturesLoaded;
}

void CBooModel::UnlockTextures() const
{
    for (TCachedToken<CTexture>& tex : *x1c_textures)
        tex.Unlock();
    ((CBooModel*)this)->x40_24_texturesLoaded = false;
}

void CBooModel::DrawAlphaSurfaces(const CModelFlags& flags) const
{
    if (TryLockTextures())
    {
        const CBooSurface* surf = x3c_firstSortedSurface;
        while (surf)
        {
            DrawSurface(*surf, flags);
            surf = surf->m_next;
        }
    }
}

void CBooModel::DrawNormalSurfaces(const CModelFlags& flags) const
{
    if (TryLockTextures())
    {
        const CBooSurface* surf = x38_firstUnsortedSurface;
        while (surf)
        {
            DrawSurface(*surf, flags);
            surf = surf->m_next;
        }
    }
}

void CBooModel::DrawSurfaces(const CModelFlags& flags) const
{
    if (!(flags.f3 & 0x4))
        if (!TryLockTextures())
            return;

    const CBooSurface* surf = x38_firstUnsortedSurface;
    while (surf)
    {
        DrawSurface(*surf, flags);
        surf = surf->m_next;
    }

    surf = x3c_firstSortedSurface;
    while (surf)
    {
        DrawSurface(*surf, flags);
        surf = surf->m_next;
    }
}

void CBooModel::DrawSurface(const CBooSurface& surf, const CModelFlags& flags) const
{
    const DataSpec::DNAMP1::HMDLMaterialSet::Material& data = GetMaterialByIndex(surf.m_data.matIdx);
    if (data.flags.shadowOccluderMesh() && !g_DrawingOccluders)
        return;

    CGraphics::SetShaderDataBinding(m_shaderDataBindings[surf.m_data.matIdx]);
    CGraphics::DrawArrayIndexed(surf.m_data.idxStart, surf.m_data.idxCount);
}

void CBooModel::DrawAlpha(const CModelFlags& flags) const
{
    DrawAlphaSurfaces(flags);
}

void CBooModel::DrawNormal(const CModelFlags& flags) const
{
    DrawNormalSurfaces(flags);
}

void CBooModel::Draw(const CModelFlags& flags) const
{
    DrawSurfaces(flags);
}

static const u8* MemoryFromPartData(const u8*& dataCur, const s32*& secSizeCur)
{
    const u8* ret;
    if (*secSizeCur)
        ret = dataCur;
    else
        ret = nullptr;

    dataCur += hecl::SBig(*secSizeCur);
    ++secSizeCur;
    return ret;
}

CModel::CModel(std::unique_ptr<u8[]>&& in, u32 dataLen, IObjectStore* store)
: x0_data(std::move(in)), x4_dataLen(dataLen)
{
    u32 version = hecl::SBig(*reinterpret_cast<u32*>(x0_data.get() + 0x4));
    u32 flags = hecl::SBig(*reinterpret_cast<u32*>(x0_data.get() + 0x8));
    if (version != 16)
        Log.report(logvisor::Fatal, "invalid CMDL for loading with boo");

    u32 secCount = hecl::SBig(*reinterpret_cast<u32*>(x0_data.get() + 0x24));
    u32 matSetCount = hecl::SBig(*reinterpret_cast<u32*>(x0_data.get() + 0x28));
    x18_matSets.reserve(matSetCount);
    const u8* dataCur = x0_data.get() + ROUND_UP_32(0x2c + secCount * 4);
    const s32* secSizeCur = reinterpret_cast<const s32*>(x0_data.get() + 0x2c);
    for (u32 i=0 ; i<matSetCount ; ++i)
    {
        u32 matSetSz = hecl::SBig(*secSizeCur);
        const u8* sec = MemoryFromPartData(dataCur, secSizeCur);
        x18_matSets.emplace_back();
        CBooModel::SShader& shader = x18_matSets.back();
        athena::io::MemoryReader r(sec, matSetSz);
        shader.m_matSet.read(r);
        CBooModel::MakeTexuresFromMats(shader.m_matSet, shader.x0_textures, *store);
    }

    hecl::HMDLMeta hmdlMeta;
    {
        u32 hmdlSz = hecl::SBig(*secSizeCur);
        const u8* hmdlMetadata = MemoryFromPartData(dataCur, secSizeCur);
        athena::io::MemoryReader r(hmdlMetadata, hmdlSz);
        hmdlMeta.read(r);
    }

    const u8* vboData = MemoryFromPartData(dataCur, secSizeCur);
    const u8* iboData = MemoryFromPartData(dataCur, secSizeCur);
    const u8* surfInfo = MemoryFromPartData(dataCur, secSizeCur);

    m_gfxToken = CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        m_vbo = ctx.newStaticBuffer(boo::BufferUse::Vertex, vboData, hmdlMeta.vertStride, hmdlMeta.vertCount);
        m_ibo = ctx.newStaticBuffer(boo::BufferUse::Index, iboData, 4, hmdlMeta.indexCount);
        m_vtxFmt = hecl::Runtime::HMDLData::NewVertexFormat(ctx, hmdlMeta, m_vbo, m_ibo);

        for (CBooModel::SShader& matSet : x18_matSets)
        {
            matSet.m_shaders.reserve(matSet.m_matSet.materials.size());
            for (const DataSpec::DNAMP1::HMDLMaterialSet::Material& mat : matSet.m_matSet.materials)
            {
                hecl::Runtime::ShaderTag tag(mat.heclIr,
                                             hmdlMeta.colorCount, hmdlMeta.uvCount, hmdlMeta.weightCount,
                                             0, mat.uvAnims.size(), true, true, true);
                matSet.m_shaders.push_back(CGraphics::g_ShaderCacheMgr->buildShader(tag, mat.heclIr, "CMDL", ctx));
            }
        }

        return true;
    });

    u32 surfCount = hecl::SBig(*reinterpret_cast<const u32*>(surfInfo));
    x8_surfaces.reserve(surfCount);
    for (u32 i=0 ; i<surfCount ; ++i)
    {
        u32 surfSz = hecl::SBig(*secSizeCur);
        const u8* sec = MemoryFromPartData(dataCur, secSizeCur);
        x8_surfaces.emplace_back();
        CBooSurface& surf = x8_surfaces.back();
        athena::io::MemoryReader r(sec, surfSz);
        surf.m_data.read(r);
    }

    const float* aabbPtr = reinterpret_cast<const float*>(x0_data.get() + 0x18);
    zeus::CAABox aabb(hecl::SBig(aabbPtr[0]), hecl::SBig(aabbPtr[1]), hecl::SBig(aabbPtr[2]),
                      hecl::SBig(aabbPtr[3]), hecl::SBig(aabbPtr[4]), hecl::SBig(aabbPtr[5]));
    x28_modelInst = std::make_unique<CBooModel>(&x8_surfaces, x18_matSets[0],
                                                m_vtxFmt, m_vbo, m_ibo,
                                                aabb, flags & 0x2, true);
}

void CBooModel::SShader::UnlockTextures()
{
    for (TCachedToken<CTexture>& tex : x0_textures)
        tex.Unlock();
}

void CModel::VerifyCurrentShader(int shaderIdx) const
{
    int idx = 0;
    for (const CBooModel::SShader& shader : x18_matSets)
        if (idx++ != shaderIdx)
            ((CBooModel::SShader&)shader).UnlockTextures();
}

void CModel::DrawSortedParts(const CModelFlags& flags) const
{
}

void CModel::DrawUnsortedParts(const CModelFlags& flags) const
{
}

void CModel::Draw(const CModelFlags& flags) const
{
}

void CModel::Touch(int shaderIdx) const
{
}

bool CModel::IsLoaded(int shaderIdx) const
{
    VerifyCurrentShader(shaderIdx);
    return false;
}

CFactoryFnReturn FModelFactory(const urde::SObjectTag& tag,
                               std::unique_ptr<u8[]>&& in, u32 len,
                               const urde::CVParamTransfer& vparms)
{
    IObjectStore* store = static_cast<TObjOwnerParam<IObjectStore*>*>(vparms.GetObj())->GetParam();
    return TToken<CModel>::GetIObjObjectFor(std::make_unique<CModel>(std::move(in), len, store));
}

}
