#include "Graphics/CModel.hpp"
#include "Graphics/CTexture.hpp"
#include "Graphics/CGraphics.hpp"
#include "Graphics/CLight.hpp"
#include "hecl/HMDLMeta.hpp"
#include "hecl/Runtime.hpp"
#include "boo/graphicsdev/Metal.hpp"
#include "Shaders/CModelShaders.hpp"
#include "Graphics/CBooRenderer.hpp"
#include "Character/CSkinRules.hpp"
#include "GameGlobalObjects.hpp"
#include <array>

namespace urde
{
static logvisor::Module Log("urde::CBooModel");
bool CBooModel::g_DrawingOccluders = false;

CBooModel::CBooModel(TToken<CModel>& token, std::vector<CBooSurface>* surfaces, SShader& shader,
                     boo::IVertexFormat* vtxFmt, boo::IGraphicsBufferS* vbo, boo::IGraphicsBufferS* ibo,
                     size_t weightVecCount, size_t skinBankCount, const zeus::CAABox& aabb)
: m_model(token), x0_surfaces(surfaces), x4_matSet(&shader.m_matSet), m_matSetIdx(shader.m_matSetIdx),
  m_pipelines(&shader.m_shaders), m_vtxFmt(vtxFmt), x8_vbo(vbo), xc_ibo(ibo), m_weightVecCount(weightVecCount),
  m_skinBankCount(skinBankCount), x1c_textures(shader.x0_textures), x20_aabb(aabb),
  x40_24_texturesLoaded(false), x40_25_modelVisible(0)
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
        /* Determine space required by uniform buffer */
        std::vector<size_t> skinOffs;
        std::vector<size_t> skinSizes;
        skinOffs.reserve(std::max(size_t(1), m_skinBankCount));
        skinSizes.reserve(std::max(size_t(1), m_skinBankCount));

        std::vector<size_t> uvOffs;
        std::vector<size_t> uvSizes;
        uvOffs.reserve(x4_matSet->materials.size());
        uvSizes.reserve(x4_matSet->materials.size());

        /* Vert transform matrices */
        size_t uniBufSize = 0;
        if (m_skinBankCount)
        {
            /* Skinned */
            for (size_t i=0 ; i<m_skinBankCount ; ++i)
            {
                size_t thisSz = ROUND_UP_256(sizeof(zeus::CMatrix4f) * (2 * m_weightVecCount * 4 + 1));
                skinOffs.push_back(uniBufSize);
                skinSizes.push_back(thisSz);
                uniBufSize += thisSz;
            }
        }
        else
        {
            /* Non-Skinned */
            size_t thisSz = ROUND_UP_256(sizeof(zeus::CMatrix4f) * 3);
            skinOffs.push_back(uniBufSize);
            skinSizes.push_back(thisSz);
            uniBufSize += thisSz;
        }

        /* Animated UV transform matrices */
        for (const MaterialSet::Material& mat : x4_matSet->materials)
        {
            size_t thisSz = ROUND_UP_256(/*mat.uvAnims.size()*/ 8 * (sizeof(zeus::CMatrix4f) * 2));
            uvOffs.push_back(uniBufSize);
            uvSizes.push_back(thisSz);
            uniBufSize += thisSz;
        }

        /* Lighting uniform */
        size_t lightOff = 0;
        size_t lightSz = 0;
        {
            size_t thisSz = ROUND_UP_256(sizeof(CModelShaders::LightingUniform));
            lightOff = uniBufSize;
            lightSz = thisSz;
            uniBufSize += thisSz;
        }

        /* Allocate resident buffer */
        m_uniformDataSize = uniBufSize;
        m_uniformBuffer = ctx.newDynamicBuffer(boo::BufferUse::Uniform, uniBufSize, 1);

        boo::IGraphicsBuffer* bufs[] = {m_uniformBuffer, m_uniformBuffer, m_uniformBuffer};

        /* Binding for each surface */
        m_shaderDataBindings.clear();
        m_shaderDataBindings.reserve(x0_surfaces->size());

        std::vector<boo::ITexture*> texs;
        size_t thisOffs[3];
        size_t thisSizes[3];

        static const boo::PipelineStage stages[3] = {boo::PipelineStage::Vertex,
                                                     boo::PipelineStage::Vertex,
                                                     boo::PipelineStage::Fragment};

        /* Enumerate surfaces and build data bindings */
        for (const CBooSurface& surf : *x0_surfaces)
        {
            const MaterialSet::Material& mat = x4_matSet->materials.at(surf.m_data.matIdx);

            texs.clear();
            texs.reserve(8);
            for (atUint32 idx : mat.textureIdxs)
            {
                TCachedToken<CTexture>& tex = x1c_textures[idx];
                texs.push_back(tex.GetObj()->GetBooTexture());
            }
            texs.resize(8);
            texs[7] = g_Renderer->x220_sphereRamp;

            if (m_skinBankCount)
            {
                thisOffs[0] = skinOffs[surf.m_data.skinMtxBankIdx];
                thisSizes[0] = skinSizes[surf.m_data.skinMtxBankIdx];
            }
            else
            {
                thisOffs[0] = 0;
                thisSizes[0] = 256;
            }

            thisOffs[1] = uvOffs[surf.m_data.matIdx];
            thisSizes[1] = uvSizes[surf.m_data.matIdx];

            thisOffs[2] = lightOff;
            thisSizes[2] = lightSz;

            const std::vector<boo::IShaderPipeline*>& pipelines = m_pipelines->at(surf.m_data.matIdx);

            m_shaderDataBindings.emplace_back();
            std::vector<boo::IShaderDataBinding*>& extendeds = m_shaderDataBindings.back();
            extendeds.reserve(pipelines.size());

            int idx = 0;
            for (boo::IShaderPipeline* pipeline : pipelines)
            {
                extendeds.push_back(
                            ctx.newShaderDataBinding(pipeline, m_vtxFmt,
                                                     x8_vbo, nullptr, xc_ibo, 3, bufs, stages,
                                                     thisOffs, thisSizes, (idx == 2) ? 8 : mat.textureIdxs.size(),
                                                     texs.data()));
                ++idx;
            }
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

void CBooModel::ActivateLights(const std::vector<CLight>& lights)
{
    m_lightingData.ambient = zeus::CColor::skBlack;
    size_t curLight = 0;

    for (const CLight& light : lights)
    {
        switch (light.x1c_type)
        {
        case ELightType::LocalAmbient:
            m_lightingData.ambient += light.x18_color;
        break;
        case ELightType::Point:
        case ELightType::Spot:
        case ELightType::Custom:
        case ELightType::Directional:
        {
            if (curLight >= URDE_MAX_LIGHTS)
                continue;
            CModelShaders::Light& lightOut = m_lightingData.lights[curLight++];
            lightOut.pos = CGraphics::g_CameraMatrix * light.x0_pos;
            lightOut.dir = CGraphics::g_CameraMatrix.basis * light.xc_dir;
            lightOut.dir.normalize();
            lightOut.color = light.x18_color;
            lightOut.linAtt[0] = light.x24_distC;
            lightOut.linAtt[1] = light.x28_distL;
            lightOut.linAtt[2] = light.x2c_distQ;
            lightOut.angAtt[0] = light.x30_angleC;
            lightOut.angAtt[1] = light.x34_angleL;
            lightOut.angAtt[2] = light.x38_angleQ;

            if (light.x1c_type == ELightType::Directional)
                lightOut.pos = (-lightOut.dir) * 1048576.f;
            break;
        }
        default: break;
        }
    }

    for (; curLight<URDE_MAX_LIGHTS ; ++curLight)
    {
        CModelShaders::Light& lightOut = m_lightingData.lights[curLight];
        lightOut.color = zeus::CColor::skClear;
        lightOut.linAtt[0] = 1.f;
        lightOut.angAtt[0] = 1.f;
    }
}

void CBooModel::RemapMaterialData(SShader& shader)
{
    x4_matSet = &shader.m_matSet;
    m_matSetIdx = shader.m_matSetIdx;
    x1c_textures = shader.x0_textures;
    m_pipelines = &shader.m_shaders;
    x40_24_texturesLoaded = false;
    m_gfxToken.doDestroy();
}

bool CBooModel::TryLockTextures() const
{
    if (!x40_24_texturesLoaded)
    {
        bool allLoad = true;
        for (TCachedToken<CTexture>& tex : const_cast<std::vector<TCachedToken<CTexture>>&>(x1c_textures))
        {
            tex.Lock();
            if (!tex.IsLoaded())
                allLoad = false;
        }

        const_cast<CBooModel*>(this)->x40_24_texturesLoaded = allLoad;
    }
    
    if (!m_gfxToken && x40_24_texturesLoaded)
        const_cast<CBooModel*>(this)->BuildGfxToken();
    
    return x40_24_texturesLoaded;
}

void CBooModel::UnlockTextures() const
{
    const_cast<boo::GraphicsDataToken&>(m_gfxToken).doDestroy();
    for (TCachedToken<CTexture>& tex : const_cast<std::vector<TCachedToken<CTexture>>&>(x1c_textures))
        tex.Unlock();
    const_cast<CBooModel*>(this)->x40_24_texturesLoaded = false;
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

    const std::vector<boo::IShaderDataBinding*>& extendeds = m_shaderDataBindings[surf.selfIdx];
    boo::IShaderDataBinding* binding = extendeds[0];
    if (flags.m_extendedShaderIdx < extendeds.size())
        binding = extendeds[flags.m_extendedShaderIdx];

    CGraphics::SetShaderDataBinding(binding);
    CGraphics::DrawArrayIndexed(surf.m_data.idxStart, surf.m_data.idxCount);
}

void CBooModel::UVAnimationBuffer::ProcessAnimation(u8*& bufOut, const UVAnimation& anim)
{
    zeus::CMatrix4f& texMtxOut = reinterpret_cast<zeus::CMatrix4f&>(*bufOut);
    zeus::CMatrix4f& postMtxOut = reinterpret_cast<zeus::CMatrix4f&>(*(bufOut + sizeof(zeus::CMatrix4f)));
    texMtxOut = zeus::CMatrix4f();
    postMtxOut = zeus::CMatrix4f();
    switch (anim.mode)
    {
    case UVAnimation::Mode::MvInvNoTranslation:
    {
        texMtxOut = CGraphics::g_GXModelViewInvXpose.toMatrix4f();
        texMtxOut.vec[3].zeroOut();
        texMtxOut.vec[3].w = 1.f;
        postMtxOut.vec[0].x = 0.5f;
        postMtxOut.vec[1].y = 0.5f;
        postMtxOut.vec[3].x = 0.5f;
        postMtxOut.vec[3].y = 0.5f;
        break;
    }
    case UVAnimation::Mode::MvInv:
    {
        texMtxOut = CGraphics::g_GXModelViewInvXpose.toMatrix4f();
        postMtxOut.vec[0].x = 0.5f;
        postMtxOut.vec[1].y = 0.5f;
        postMtxOut.vec[3].x = 0.5f;
        postMtxOut.vec[3].y = 0.5f;
        break;
    }
    case UVAnimation::Mode::Scroll:
    {
        texMtxOut.vec[3].x = CGraphics::GetSecondsMod900() * anim.vals[2] + anim.vals[0];
        texMtxOut.vec[3].y = CGraphics::GetSecondsMod900() * anim.vals[3] + anim.vals[1];
        break;
    }
    case UVAnimation::Mode::Rotation:
    {
        float angle = CGraphics::GetSecondsMod900() * anim.vals[1] + anim.vals[0];
        float acos = std::cos(angle);
        float asin = std::sin(angle);
        texMtxOut.vec[0].x = acos;
        texMtxOut.vec[0].y = asin;
        texMtxOut.vec[1].x = -asin;
        texMtxOut.vec[1].y = acos;
        texMtxOut.vec[3].x = (1.0f - (acos - asin)) * 0.5f;
        texMtxOut.vec[3].y = (1.0f - (asin + acos)) * 0.5f;
        break;
    }
    case UVAnimation::Mode::HStrip:
    {
        float value = anim.vals[0] * anim.vals[2] * (anim.vals[3] + CGraphics::GetSecondsMod900());
        texMtxOut.vec[3].x = (float)(short)(float)(anim.vals[1] * fmod(value, 1.0f)) * anim.vals[2];
        break;
    }
    case UVAnimation::Mode::VStrip:
    {
        float value = anim.vals[0] * anim.vals[2] * (anim.vals[3] + CGraphics::GetSecondsMod900());
        texMtxOut.vec[3].y = (float)(short)(float)(anim.vals[1] * fmod(value, 1.0f)) * anim.vals[2];
        break;
    }
    case UVAnimation::Mode::Model:
    {
        texMtxOut = CGraphics::g_GXModelMatrix.toMatrix4f();
        texMtxOut.vec[3].zeroOut();

        postMtxOut.vec[0].x = 0.5f;
        postMtxOut.vec[2].y = 0.5f;
        postMtxOut.vec[3].x = CGraphics::g_GXModelMatrix.origin.x * 0.5f;
        postMtxOut.vec[3].y = CGraphics::g_GXModelMatrix.origin.y * 0.5f;
        break;
    }
    case UVAnimation::Mode::CylinderEnvironment:
    {
        texMtxOut = (CGraphics::g_ViewMatrix.inverse() * CGraphics::g_GXModelMatrix).toMatrix4f();
        texMtxOut.vec[3].zeroOut();

        const zeus::CVector3f& viewOrigin = CGraphics::g_ViewMatrix.origin;
        float xy = (viewOrigin.x + viewOrigin.y) * 0.025f * anim.vals[1];
        xy = (xy - (int)xy);
        float z = (viewOrigin.z) * 0.05f * anim.vals[1];
        z = (z - (int)z);

        float halfA = anim.vals[0] * 0.5f;

        postMtxOut = zeus::CTransform(zeus::CMatrix3f(halfA, 0.0, 0.0,
                                                   0.0, 0.0, halfA,
                                                   0.0, 0.0, 0.0),
                                   zeus::CVector3f(xy, z, 1.0)).toMatrix4f();
        break;
    }
    default: break;
    }
    bufOut += sizeof(zeus::CMatrix4f) * 2;
}

void CBooModel::UVAnimationBuffer::PadOutBuffer(u8*& bufStart, u8*& bufOut)
{
    bufOut = bufStart + ROUND_UP_256(bufOut - bufStart);
}

void CBooModel::UVAnimationBuffer::Update(u8*& bufOut, const MaterialSet* matSet, const CModelFlags& flags)
{
    u8* start = bufOut;

    /* Special Mode0 matrix for exclusive Thermal Visor use */
    std::experimental::optional<std::array<zeus::CMatrix4f, 2>> thermalMtxOut;
    if (flags.m_extendedShaderIdx == 2)
    {
        thermalMtxOut.emplace();

        zeus::CMatrix4f& texMtxOut = (*thermalMtxOut)[0];
        texMtxOut = CGraphics::g_GXModelViewInvXpose.toMatrix4f();
        texMtxOut.vec[3].zeroOut();
        texMtxOut.vec[3].w = 1.f;

        zeus::CMatrix4f& postMtxOut = (*thermalMtxOut)[1];
        postMtxOut.vec[0].x = 0.5f;
        postMtxOut.vec[1].y = 0.5f;
        postMtxOut.vec[3].x = 0.5f;
        postMtxOut.vec[3].y = 0.5f;
    }

    for (const MaterialSet::Material& mat : matSet->materials)
    {
        if (thermalMtxOut)
        {
            std::array<zeus::CMatrix4f, 2>* mtxs = reinterpret_cast<std::array<zeus::CMatrix4f, 2>*>(bufOut);
            mtxs[7][0] = (*thermalMtxOut)[0];
            mtxs[7][1] = (*thermalMtxOut)[1];
        }
        u8* bufOrig = bufOut;
        for (const UVAnimation& anim : mat.uvAnims)
            ProcessAnimation(bufOut, anim);
        bufOut = bufOrig + sizeof(zeus::CMatrix4f) * 2 * 8;
        PadOutBuffer(start, bufOut);
    }
}

void CBooModel::UpdateUniformData(const CModelFlags& flags,
                                  const CSkinRules* cskr,
                                  const CPoseAsTransforms* pose) const
{
    if (!m_uniformBuffer)
        return;

    u8* dataOut = reinterpret_cast<u8*>(m_uniformBuffer->map(m_uniformDataSize));
    u8* dataCur = dataOut;

    if (m_skinBankCount)
    {
        /* Skinned */
        std::vector<const zeus::CTransform*> bankTransforms;
        size_t weightCount = m_weightVecCount * 4;
        bankTransforms.reserve(weightCount);
        for (size_t i=0 ; i<m_skinBankCount ; ++i)
        {
            if (cskr && pose)
            {
                cskr->GetBankTransforms(bankTransforms, *pose, i);

                for (size_t w=0 ; w<weightCount ; ++w)
                {
                    zeus::CMatrix4f& mv = reinterpret_cast<zeus::CMatrix4f&>(*dataCur);
                    if (w >= bankTransforms.size())
                        mv = CGraphics::g_GXModelView.toMatrix4f();
                    else
                        mv = (CGraphics::g_GXModelView * *bankTransforms[w]).toMatrix4f();
                    dataCur += sizeof(zeus::CMatrix4f);
                }
                for (size_t w=0 ; w<weightCount ; ++w)
                {
                    zeus::CMatrix4f& mvinv = reinterpret_cast<zeus::CMatrix4f&>(*dataCur);
                    if (w >= bankTransforms.size())
                        mvinv = CGraphics::g_GXModelViewInvXpose.toMatrix4f();
                    else
                    {
                        zeus::CTransform xf = (CGraphics::g_GXModelView.basis * bankTransforms[w]->basis);
                        xf.basis.invert();
                        xf.basis.transpose();
                        mvinv = xf.toMatrix4f();
                    }
                    dataCur += sizeof(zeus::CMatrix4f);
                }

                bankTransforms.clear();
            }
            else
            {
                for (size_t w=0 ; w<weightCount ; ++w)
                {
                    zeus::CMatrix4f& mv = reinterpret_cast<zeus::CMatrix4f&>(*dataCur);
                    mv = CGraphics::g_GXModelView.toMatrix4f();
                    dataCur += sizeof(zeus::CMatrix4f);
                }
                for (size_t w=0 ; w<weightCount ; ++w)
                {
                    zeus::CMatrix4f& mvinv = reinterpret_cast<zeus::CMatrix4f&>(*dataCur);
                    mvinv = CGraphics::g_GXModelViewInvXpose.toMatrix4f();
                    dataCur += sizeof(zeus::CMatrix4f);
                }
            }
            zeus::CMatrix4f& proj = reinterpret_cast<zeus::CMatrix4f&>(*dataCur);
            proj = CGraphics::GetPerspectiveProjectionMatrix(true);
            dataCur += sizeof(zeus::CMatrix4f);

            dataCur = dataOut + ROUND_UP_256(dataCur - dataOut);
        }
    }
    else
    {
        /* Non-Skinned */
        zeus::CMatrix4f& mv = reinterpret_cast<zeus::CMatrix4f&>(*dataCur);
        mv = CGraphics::g_GXModelView.toMatrix4f();
        dataCur += sizeof(zeus::CMatrix4f);

        zeus::CMatrix4f& mvinv = reinterpret_cast<zeus::CMatrix4f&>(*dataCur);
        mvinv = CGraphics::g_GXModelViewInvXpose.toMatrix4f();
        dataCur += sizeof(zeus::CMatrix4f);

        zeus::CMatrix4f& proj = reinterpret_cast<zeus::CMatrix4f&>(*dataCur);
        proj = CGraphics::GetPerspectiveProjectionMatrix(true);
        dataCur += sizeof(zeus::CMatrix4f);

        dataCur = dataOut + ROUND_UP_256(dataCur - dataOut);
    }

    UVAnimationBuffer::Update(dataCur, x4_matSet, flags);

    if (flags.m_extendedShaderIdx == 2) /* Thermal Model (same as UV Mode 0) */
    {
        CModelShaders::ThermalUniform& thermalOut = *reinterpret_cast<CModelShaders::ThermalUniform*>(dataCur);
        thermalOut.mulColor = flags.color;
        thermalOut.addColor = flags.addColor;
    }
    else
    {
        CModelShaders::LightingUniform& lightingOut = *reinterpret_cast<CModelShaders::LightingUniform*>(dataCur);
        lightingOut = m_lightingData;
        lightingOut.colorRegs[0] = flags.regColors[0];
        lightingOut.colorRegs[1] = flags.regColors[1];
        lightingOut.colorRegs[2] = flags.regColors[2];
        lightingOut.fog = CGraphics::g_Fog;
    }

    m_uniformBuffer->unmap();
 }

void CBooModel::DrawAlpha(const CModelFlags& flags,
                          const CSkinRules* cskr,
                          const CPoseAsTransforms* pose) const
{
    if (TryLockTextures())
    {
        UpdateUniformData(flags, cskr, pose);
        DrawAlphaSurfaces(flags);
    }
}

void CBooModel::DrawNormal(const CModelFlags& flags,
                           const CSkinRules* cskr,
                           const CPoseAsTransforms* pose) const
{
    if (TryLockTextures())
    {
        UpdateUniformData(flags, cskr, pose);
        DrawNormalSurfaces(flags);
    }
}

void CBooModel::Draw(const CModelFlags& flags,
                     const CSkinRules* cskr,
                     const CPoseAsTransforms* pose) const
{
    if (TryLockTextures())
    {
        UpdateUniformData(flags, cskr, pose);
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

std::unique_ptr<CBooModel> CModel::MakeNewInstance(int shaderIdx)
{
    if (shaderIdx >= x18_matSets.size())
        shaderIdx = 0;
    return std::make_unique<CBooModel>(m_selfToken, &x8_surfaces, x18_matSets[shaderIdx],
                                       m_vtxFmt, m_vbo, m_ibo, m_weightVecCount, m_skinBankCount, m_aabb);
}

CModel::CModel(std::unique_ptr<u8[]>&& in, u32 /* dataLen */, IObjectStore* store, CObjectReference* selfRef)
: m_selfToken(selfRef)
{
    std::unique_ptr<u8[]> data = std::move(in);

    u32 version = hecl::SBig(*reinterpret_cast<u32*>(data.get() + 0x4));
    u32 flags = hecl::SBig(*reinterpret_cast<u32*>(data.get() + 0x8));
    if (version != 0x10002)
        Log.report(logvisor::Fatal, "invalid CMDL for loading with boo");

    u32 secCount = hecl::SBig(*reinterpret_cast<u32*>(data.get() + 0x24));
    u32 matSetCount = hecl::SBig(*reinterpret_cast<u32*>(data.get() + 0x28));
    x18_matSets.reserve(matSetCount);
    const u8* dataCur = data.get() + ROUND_UP_32(0x2c + secCount * 4);
    const s32* secSizeCur = reinterpret_cast<const s32*>(data.get() + 0x2c);
    for (u32 i=0 ; i<matSetCount ; ++i)
    {
        u32 matSetSz = hecl::SBig(*secSizeCur);
        const u8* sec = MemoryFromPartData(dataCur, secSizeCur);
        x18_matSets.emplace_back(i);
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
    m_weightVecCount = hmdlMeta.weightCount;
    m_skinBankCount = hmdlMeta.bankCount;

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
                                             hmdlMeta.weightCount * 4, 8, boo::Primitive(hmdlMeta.topology),
                                             true, true, true);
                matSet.m_shaders.push_back(CModelShaders::g_ModelShaders->buildExtendedShader(tag, mat.heclIr, "CMDL", ctx));
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
        surf.selfIdx = i;
        athena::io::MemoryReader r(sec, surfSz);
        surf.m_data.read(r);
    }

    const float* aabbPtr = reinterpret_cast<const float*>(data.get() + 0xc);
    m_aabb = zeus::CAABox(hecl::SBig(aabbPtr[0]), hecl::SBig(aabbPtr[1]), hecl::SBig(aabbPtr[2]),
            hecl::SBig(aabbPtr[3]), hecl::SBig(aabbPtr[4]), hecl::SBig(aabbPtr[5]));
    x28_modelInst = MakeNewInstance(0);
}

void CBooModel::SShader::UnlockTextures()
{
    for (TCachedToken<CTexture>& tex : x0_textures)
        tex.Unlock();
}

void CBooModel::VerifyCurrentShader(int shaderIdx)
{
    if (shaderIdx != m_matSetIdx)
        RemapMaterialData(m_model->x18_matSets[shaderIdx]);
}

void CBooModel::Touch(int shaderIdx) const
{
    const_cast<CBooModel*>(this)->VerifyCurrentShader(shaderIdx);
    TryLockTextures();
}

void CModel::DrawSortedParts(const CModelFlags& flags) const
{
    const_cast<CBooModel&>(*x28_modelInst).VerifyCurrentShader(flags.m_matSetIdx);
    x28_modelInst->DrawAlpha(flags, nullptr, nullptr);
}

void CModel::DrawUnsortedParts(const CModelFlags& flags) const
{
    const_cast<CBooModel&>(*x28_modelInst).VerifyCurrentShader(flags.m_matSetIdx);
    x28_modelInst->DrawNormal(flags, nullptr, nullptr);
}

void CModel::Draw(const CModelFlags& flags) const
{
    const_cast<CBooModel&>(*x28_modelInst).VerifyCurrentShader(flags.m_matSetIdx);
    x28_modelInst->Draw(flags, nullptr, nullptr);
}

bool CModel::IsLoaded(int shaderIdx) const
{
    const_cast<CBooModel&>(*x28_modelInst).VerifyCurrentShader(shaderIdx);
    std::vector<TCachedToken<CTexture>>& texs = x28_modelInst->x1c_textures;
    bool loaded = true;
    for (TCachedToken<CTexture>& tex : texs)
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
                               const urde::CVParamTransfer& vparms,
                               CObjectReference* selfRef)
{
    IObjectStore* store = static_cast<TObjOwnerParam<IObjectStore*>*>(vparms.GetObj())->GetParam();
    CFactoryFnReturn ret = TToken<CModel>::GetIObjObjectFor(std::make_unique<CModel>(std::move(in), len, store, selfRef));
    return ret;
}

}
