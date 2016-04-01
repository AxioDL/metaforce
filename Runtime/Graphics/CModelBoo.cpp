#include "Graphics/CModel.hpp"
#include "Graphics/CTexture.hpp"
#include "Graphics/CGraphics.hpp"
#include "hecl/HMDLMeta.hpp"
#include "hecl/Runtime.hpp"

namespace urde
{
static logvisor::Module Log("urde::CModelBoo");
bool CBooModel::g_DrawingOccluders = false;

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
        const MaterialSet::Material& matData = GetMaterialByIndex(matId);
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
    m_gfxToken = CGraphics::CommitResources(
    [&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        m_unskinnedXfBuffer = ctx.newDynamicBuffer(boo::BufferUse::Vertex,
                                               sizeof(SUnskinnedXf), 1);
        boo::IGraphicsBuffer* bufs[] = {m_unskinnedXfBuffer};
        m_shaderDataBindings.clear();
        m_shaderDataBindings.reserve(x4_matSet->materials.size());
        auto pipelineIt = m_pipelines->begin();
        std::vector<boo::ITexture*> texs;
        for (const MaterialSet::Material& mat : x4_matSet->materials)
        {
            texs.clear();
            texs.reserve(mat.textureIdxs.size());
            for (atUint32 idx : mat.textureIdxs)
            {
                TCachedToken<CTexture>& tex = (*x1c_textures)[idx];
                texs.push_back(tex.GetObj()->GetBooTexture());
            }
            m_shaderDataBindings.push_back(
                ctx.newShaderDataBinding(*pipelineIt, m_vtxFmt,
                                         x8_vbo, nullptr, xc_ibo, 1, bufs,
                                         mat.textureIdxs.size(), texs.data()));
            ++pipelineIt;
        }
        return true;
    });
}

void CBooModel::MakeTexuresFromMats(const MaterialSet& matSet,
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
    const CBooSurface* surf = x3c_firstSortedSurface;
    while (surf)
    {
        DrawSurface(*surf, flags);
        surf = surf->m_next;
    }
}

void CBooModel::DrawNormalSurfaces(const CModelFlags& flags) const
{
    const CBooSurface* surf = x38_firstUnsortedSurface;
    while (surf)
    {
        DrawSurface(*surf, flags);
        surf = surf->m_next;
    }
}

void CBooModel::DrawSurfaces(const CModelFlags& flags) const
{
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
    const MaterialSet::Material& data = GetMaterialByIndex(surf.m_data.matIdx);
    if (data.flags.shadowOccluderMesh() && !g_DrawingOccluders)
        return;

    CGraphics::SetShaderDataBinding(m_shaderDataBindings[surf.m_data.matIdx]);
    CGraphics::DrawArrayIndexed(surf.m_data.idxStart, surf.m_data.idxCount);
}

void CBooModel::UVAnimationBuffer::ProcessAnimation(const UVAnimation& anim)
{
    m_buffer.emplace_back();
    zeus::CMatrix4f& matrixOut = m_buffer.back();
    switch (anim.mode)
    {
    case UVAnimation::Mode::MvInvNoTranslation:
    {
        matrixOut = CGraphics::g_ViewMatrix.inverse().multiplyIgnoreTranslation(
                        CGraphics::g_GXModelMatrix).toMatrix4f();
        matrixOut.vec[3].zeroOut();
        break;
    }
    case UVAnimation::Mode::MvInv:
    {
        matrixOut = (CGraphics::g_ViewMatrix.inverse() * CGraphics::g_GXModelMatrix).toMatrix4f();
        break;
    }
    case UVAnimation::Mode::Scroll:
    {
        matrixOut.vec[3].x = CGraphics::GetSecondsMod900() * anim.vals[2] + anim.vals[0];
        matrixOut.vec[3].y = CGraphics::GetSecondsMod900() * anim.vals[3] + anim.vals[1];
        break;
    }
    case UVAnimation::Mode::Rotation:
    {
        float angle = CGraphics::GetSecondsMod900() * anim.vals[1] + anim.vals[0];
        float acos = std::cos(angle);
        float asin = std::sin(angle);
        matrixOut.vec[0].x = acos;
        matrixOut.vec[0].y = asin;
        matrixOut.vec[1].x = -asin;
        matrixOut.vec[1].y = acos;
        matrixOut.vec[3].x = (1.0 - (acos - asin)) * 0.5;
        matrixOut.vec[3].y = (1.0 - (asin + acos)) * 0.5;
        break;
    }
    case UVAnimation::Mode::HStrip:
    {
        float value = anim.vals[2] * anim.vals[0] * (anim.vals[3] + CGraphics::GetSecondsMod900());
        matrixOut.vec[3].x = anim.vals[1] * fmod(value, 1.0f) * anim.vals[2];
        break;
    }
    case UVAnimation::Mode::VStrip:
    {
        float value = anim.vals[2] * anim.vals[0] * (anim.vals[3] + CGraphics::GetSecondsMod900());
        matrixOut.vec[3].y = anim.vals[1] * fmod(value, 1.0f) * anim.vals[2];
        break;
    }
    case UVAnimation::Mode::Model:
    {
        matrixOut.vec[0].x = 0.5f;
        matrixOut.vec[1].y = 0.0f;
        matrixOut.vec[2].y = 0.5f;
        matrixOut.vec[3].x = CGraphics::g_GXModelMatrix.m_origin.x * 0.5f;
        matrixOut.vec[3].y = CGraphics::g_GXModelMatrix.m_origin.y * 0.5f;
        break;
    }
    case UVAnimation::Mode::WhoMustNotBeNamed:
    {
        zeus::CTransform texmtx = CGraphics::g_ViewMatrix.inverse() * CGraphics::g_GXModelMatrix;
        texmtx.m_origin.zeroOut();
        /* TODO: Finish */
        matrixOut = texmtx.toMatrix4f();
        break;
    }
    default: break;
    }
}

void CBooModel::UVAnimationBuffer::PadOutBuffer()
{
    size_t curEnd = 0;
    if (m_ranges.size())
        curEnd = m_ranges.back().first + m_ranges.back().second;

    size_t bufRem = m_buffer.size() % 4;
    if (bufRem)
        for (int i=0 ; i<(4-bufRem) ; ++i)
            m_buffer.emplace_back();
    size_t newEnd = m_buffer.size() * 64;

    m_ranges.emplace_back(curEnd, newEnd - curEnd);
}

void CBooModel::UVAnimationBuffer::Update(const MaterialSet* matSet)
{
    m_buffer.clear();
    m_ranges.clear();

    size_t bufCount = 0;
    size_t count = 0;
    for (const MaterialSet::Material& mat : matSet->materials)
    {
        count += mat.uvAnims.size();
        bufCount += mat.uvAnims.size();
        bufCount = ROUND_UP_4(bufCount);
    }
    m_buffer.reserve(bufCount);
    m_ranges.reserve(count);

    for (const MaterialSet::Material& mat : matSet->materials)
    {
        for (const UVAnimation& anim : mat.uvAnims)
            ProcessAnimation(anim);
        PadOutBuffer();
    }
}

void CBooModel::UpdateUniformData() const
{
    SUnskinnedXf unskinnedXf;
    unskinnedXf.mv = CGraphics::g_GXModelView.toMatrix4f();
    unskinnedXf.mvinv = CGraphics::g_GXModelViewInvXpose.toMatrix4f();
    unskinnedXf.proj = CGraphics::GetPerspectiveProjectionMatrix();
    m_unskinnedXfBuffer->load(&unskinnedXf, sizeof(unskinnedXf));

    if (m_uvAnimBuffer)
    {
        ((CBooModel*)this)->m_uvAnimBuffer.Update(x4_matSet);
        m_uvMtxBuffer->load(m_uvAnimBuffer.m_buffer.data(),
                            m_uvAnimBuffer.m_buffer.size() * 64);
    }
}

void CBooModel::DrawAlpha(const CModelFlags& flags) const
{
    if (TryLockTextures())
    {
        UpdateUniformData();
        DrawAlphaSurfaces(flags);
    }
}

void CBooModel::DrawNormal(const CModelFlags& flags) const
{
    if (TryLockTextures())
    {
        UpdateUniformData();
        DrawNormalSurfaces(flags);
    }
}

void CBooModel::Draw(const CModelFlags& flags) const
{
    if (TryLockTextures())
    {
        UpdateUniformData();
        DrawSurfaces(flags);
    }
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
    if (version != 0x10002)
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
            for (const MaterialSet::Material& mat : matSet.m_matSet.materials)
            {
                hecl::Runtime::ShaderTag tag(mat.heclIr,
                                             hmdlMeta.colorCount, hmdlMeta.uvCount, hmdlMeta.weightCount,
                                             0, mat.uvAnims.size(), false, false, true);
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

    const float* aabbPtr = reinterpret_cast<const float*>(x0_data.get() + 0xc);
    zeus::CAABox aabb(hecl::SBig(aabbPtr[0]), hecl::SBig(aabbPtr[1]), hecl::SBig(aabbPtr[2]),
                      hecl::SBig(aabbPtr[3]), hecl::SBig(aabbPtr[4]), hecl::SBig(aabbPtr[5]));
    x28_modelInst = std::make_unique<CBooModel>(&x8_surfaces, x18_matSets[0],
                                                m_vtxFmt, m_vbo, m_ibo,
                                                aabb, flags & 0x2, false);
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
    VerifyCurrentShader(flags.m_matSetIdx);
    x28_modelInst->DrawAlpha(flags);
}

void CModel::DrawUnsortedParts(const CModelFlags& flags) const
{
    VerifyCurrentShader(flags.m_matSetIdx);
    x28_modelInst->DrawNormal(flags);
}

void CModel::Draw(const CModelFlags& flags) const
{
    VerifyCurrentShader(flags.m_matSetIdx);
    x28_modelInst->Draw(flags);
}

void CModel::Touch(int shaderIdx) const
{
    VerifyCurrentShader(shaderIdx);
    x28_modelInst->TryLockTextures();
}

bool CModel::IsLoaded(int shaderIdx) const
{
    VerifyCurrentShader(shaderIdx);
    std::vector<TCachedToken<CTexture>>* texs = x28_modelInst->x1c_textures;
    bool loaded = true;
    for (TCachedToken<CTexture>& tex : *texs)
    {
        if (!tex.IsLoaded())
        {
            loaded = false;
            break;
        }
    }
    return loaded;
}

CFactoryFnReturn FModelFactory(const urde::SObjectTag& tag,
                               std::unique_ptr<u8[]>&& in, u32 len,
                               const urde::CVParamTransfer& vparms)
{
    IObjectStore* store = static_cast<TObjOwnerParam<IObjectStore*>*>(vparms.GetObj())->GetParam();
    CFactoryFnReturn ret = TToken<CModel>::GetIObjObjectFor(std::make_unique<CModel>(std::move(in), len, store));
    return ret;
}

}
