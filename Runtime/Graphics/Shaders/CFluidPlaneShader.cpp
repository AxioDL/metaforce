#include "CFluidPlaneShader.hpp"

namespace urde
{

CFluidPlaneShader::Cache CFluidPlaneShader::_cache = {};

u16 CFluidPlaneShader::Cache::MakeCacheKey(const SFluidPlaneShaderInfo& info)
{
    u16 ret = 0;

    switch (info.m_type)
    {
    case CFluidPlane::EFluidType::NormalWater:
    case CFluidPlane::EFluidType::Three:
    case CFluidPlane::EFluidType::Four:
        if (info.m_hasLightmap)
        {
            ret |= 1 << 2;
            if (info.m_doubleLightmapBlend)
                ret |= 1 << 3;
        }

        if (!info.m_hasEnvMap && info.m_hasEnvBumpMap)
            ret |= 1 << 4;

        if (info.m_hasEnvMap)
            ret |= 1 << 5;

        break;

    case CFluidPlane::EFluidType::PoisonWater:
        ret |= 1;

        if (info.m_hasLightmap)
        {
            ret |= 1 << 2;
            if (info.m_doubleLightmapBlend)
                ret |= 1 << 3;
        }

        if (info.m_hasEnvBumpMap)
            ret |= 1 << 4;

        break;

    case CFluidPlane::EFluidType::Lava:
        ret |= 2;

        if (info.m_hasBumpMap)
            ret |= 1 << 2;

        break;

    case CFluidPlane::EFluidType::Five:
        ret |= 3;

        if (info.m_hasBumpMap)
            ret |= 1 << 2;

        break;
    }

    if (info.m_hasPatternTex1)
        ret |= 1 << 6;
    if (info.m_hasPatternTex2)
        ret |= 1 << 7;
    if (info.m_hasColorTex)
        ret |= 1 << 8;

    if (info.m_additive)
        ret |= 1 << 9;

    return ret;
}

boo::IShaderPipeline* CFluidPlaneShader::Cache::GetOrBuildShader(const SFluidPlaneShaderInfo& info)
{
    u16 key = MakeCacheKey(info);
    auto& slot = m_cache[key];
    if (slot.second != nullptr)
        return slot.second;

    if (CGraphics::g_BooFactory == nullptr)
        return nullptr;

    slot.first = CGraphics::CommitResources(
    [&](boo::IGraphicsDataFactory::Context& ctx)
    {
        switch (ctx.platform())
        {
        case boo::IGraphicsDataFactory::Platform::OpenGL:
            slot.second = BuildShader(static_cast<boo::GLDataFactory::Context&>(ctx), info);
            break;
#if _WIN32
        case boo::IGraphicsDataFactory::Platform::D3D11:
        case boo::IGraphicsDataFactory::Platform::D3D12:
            slot.second = BuildShader(static_cast<boo::ID3DDataFactory::Context&>(ctx), info);
            break;
#endif
#if BOO_HAS_METAL
        case boo::IGraphicsDataFactory::Platform::Metal:
            slot.second = BuildShader(static_cast<boo::MetalDataFactory::Context&>(ctx), info);
            break;
#endif
#if BOO_HAS_VULKAN
        case boo::IGraphicsDataFactory::Platform::Vulkan:
            slot.second = BuildShader(static_cast<boo::VulkanDataFactory::Context&>(ctx), info);
            break;
#endif
        default: break;
        }
        return true;
    });

    return slot.second;
}

CFluidPlaneShader::CFluidPlaneShader(CFluidPlane::EFluidType type,
                                     const std::experimental::optional<TLockedToken<CTexture>>& patternTex1,
                                     const std::experimental::optional<TLockedToken<CTexture>>& patternTex2,
                                     const std::experimental::optional<TLockedToken<CTexture>>& colorTex,
                                     const std::experimental::optional<TLockedToken<CTexture>>& bumpMap,
                                     const std::experimental::optional<TLockedToken<CTexture>>& envMap,
                                     const std::experimental::optional<TLockedToken<CTexture>>& envBumpMap,
                                     const std::experimental::optional<TLockedToken<CTexture>>& lightmap,
                                     bool doubleLightmapBlend, bool additive)
: m_patternTex1(patternTex1),
  m_patternTex2(patternTex2),
  m_colorTex(colorTex),
  m_bumpMap(bumpMap),
  m_envMap(envMap),
  m_envBumpMap(envBumpMap),
  m_lightmap(lightmap)
{
    SFluidPlaneShaderInfo shaderInfo(type,
                                     m_patternTex1.operator bool(),
                                     m_patternTex2.operator bool(),
                                     m_colorTex.operator bool(),
                                     m_bumpMap.operator bool(),
                                     m_envMap.operator bool(),
                                     m_envBumpMap.operator bool(),
                                     m_lightmap.operator bool(),
                                     doubleLightmapBlend, additive);
    boo::IShaderPipeline* pipeline = _cache.GetOrBuildShader(shaderInfo);

    m_gfxTok = CGraphics::CommitResources(
    [&](boo::IGraphicsDataFactory::Context& ctx)
    {
        m_vbo = ctx.newDynamicBuffer(boo::BufferUse::Vertex, sizeof(Vertex), 999); // TODO: Figure out how many
        m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, 1024, 1);

        switch (ctx.platform())
        {
        case boo::IGraphicsDataFactory::Platform::OpenGL:
            m_dataBind = BuildBinding(static_cast<boo::GLDataFactory::Context&>(ctx), pipeline);
            break;
#if _WIN32
        case boo::IGraphicsDataFactory::Platform::D3D11:
        case boo::IGraphicsDataFactory::Platform::D3D12:
            m_dataBind = BuildBinding(static_cast<boo::ID3DDataFactory::Context&>(ctx), pipeline);
            break;
#endif
#if BOO_HAS_METAL
        case boo::IGraphicsDataFactory::Platform::Metal:
            m_dataBind = BuildBinding(static_cast<boo::MetalDataFactory::Context&>(ctx), pipeline);
            break;
#endif
#if BOO_HAS_VULKAN
        case boo::IGraphicsDataFactory::Platform::Vulkan:
            m_dataBind = BuildBinding(static_cast<boo::VulkanDataFactory::Context&>(ctx), pipeline);
            break;
#endif
        default: break;
        }
        return true;
    });
}

void CFluidPlaneShader::draw(const zeus::CMatrix4f texMtxs[6], const zeus::CMatrix4f& normMtx, float indScale,
                             const std::vector<CLight>& lights, const zeus::CColor kColors[4],
                             const std::vector<Vertex>& verts)
{
    Uniform& uni = *reinterpret_cast<Uniform*>(m_uniBuf->map(sizeof(Uniform)));
    uni.m_mv = CGraphics::g_GXModelView.toMatrix4f();
    uni.m_mvNorm = normMtx;
    uni.m_proj = CGraphics::GetPerspectiveProjectionMatrix(true);
    for (int i=0 ; i<6 ; ++i)
        uni.m_texMtxs[i] = texMtxs[i];
    uni.m_lighting.ActivateLights(lights);
    for (int i=0 ; i<3 ; ++i)
        uni.m_lighting.colorRegs[i] = kColors[i];
    uni.m_lighting.mulColor = kColors[3];
    uni.m_lighting.fog.m_rangeScale = indScale;
    m_uniBuf->unmap();

    m_vbo->load(verts.data(), verts.size() * sizeof(Vertex));

    CGraphics::SetShaderDataBinding(m_dataBind);
    CGraphics::DrawArray(0, verts.size());
}

}
