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
    case CFluidPlane::EFluidType::PhazonFluid:
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

    case CFluidPlane::EFluidType::ThickLava:
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

u16 CFluidPlaneShader::Cache::MakeCacheKey(const SFluidPlaneDoorShaderInfo& info)
{
    u16 ret = 0;

    if (info.m_hasPatternTex1)
        ret |= 1 << 0;
    if (info.m_hasPatternTex2)
        ret |= 1 << 1;
    if (info.m_hasColorTex)
        ret |= 1 << 2;

    return ret;
}

template<class T>
boo::ObjToken<boo::IShaderPipeline> CFluidPlaneShader::Cache::GetOrBuildShader(const T& info)
{
    u16 key = MakeCacheKey(info);
    auto& slot = CacheSlot(info, key);
    if (slot)
        return slot;

    if (CGraphics::g_BooFactory == nullptr)
        return nullptr;

    CGraphics::CommitResources(
    [&](boo::IGraphicsDataFactory::Context& ctx)
    {
        switch (ctx.platform())
        {
        case boo::IGraphicsDataFactory::Platform::OpenGL:
            slot = BuildShader(static_cast<boo::GLDataFactory::Context&>(ctx), info);
            break;
#if _WIN32
        case boo::IGraphicsDataFactory::Platform::D3D11:
        case boo::IGraphicsDataFactory::Platform::D3D12:
            slot = BuildShader(static_cast<boo::ID3DDataFactory::Context&>(ctx), info);
            break;
#endif
#if BOO_HAS_METAL
        case boo::IGraphicsDataFactory::Platform::Metal:
            slot = BuildShader(static_cast<boo::MetalDataFactory::Context&>(ctx), info);
            break;
#endif
#if BOO_HAS_VULKAN
        case boo::IGraphicsDataFactory::Platform::Vulkan:
            slot = BuildShader(static_cast<boo::VulkanDataFactory::Context&>(ctx), info);
            break;
#endif
        default: break;
        }
        return true;
    });

    return slot;
}

template boo::ObjToken<boo::IShaderPipeline>
CFluidPlaneShader::Cache::GetOrBuildShader<SFluidPlaneShaderInfo>(const SFluidPlaneShaderInfo& info);
template boo::ObjToken<boo::IShaderPipeline>
CFluidPlaneShader::Cache::GetOrBuildShader<SFluidPlaneDoorShaderInfo>(const SFluidPlaneDoorShaderInfo& info);

void CFluidPlaneShader::Cache::Clear()
{
    for (auto& p : m_cache)
        p.reset();
    for (auto& p : m_doorCache)
        p.reset();
}

void CFluidPlaneShader::PrepareBinding(const boo::ObjToken<boo::IShaderPipeline>& pipeline, u32 maxVertCount, bool door)
{
    CGraphics::CommitResources(
    [&](boo::IGraphicsDataFactory::Context& ctx)
    {
        m_vbo = ctx.newDynamicBuffer(boo::BufferUse::Vertex, sizeof(Vertex), maxVertCount);
        m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, 1024, 1);

        switch (ctx.platform())
        {
        case boo::IGraphicsDataFactory::Platform::OpenGL:
            m_dataBind = BuildBinding(static_cast<boo::GLDataFactory::Context&>(ctx), pipeline, door);
            break;
#if _WIN32
        case boo::IGraphicsDataFactory::Platform::D3D11:
        case boo::IGraphicsDataFactory::Platform::D3D12:
            m_dataBind = BuildBinding(static_cast<boo::ID3DDataFactory::Context&>(ctx), pipeline, door);
            break;
#endif
#if BOO_HAS_METAL
        case boo::IGraphicsDataFactory::Platform::Metal:
            m_dataBind = BuildBinding(static_cast<boo::MetalDataFactory::Context&>(ctx), pipeline, door);
            break;
#endif
#if BOO_HAS_VULKAN
        case boo::IGraphicsDataFactory::Platform::Vulkan:
            m_dataBind = BuildBinding(static_cast<boo::VulkanDataFactory::Context&>(ctx), pipeline, door);
            break;
#endif
        default: break;
        }
        return true;
    });
}

CFluidPlaneShader::CFluidPlaneShader(CFluidPlane::EFluidType type,
                                     const std::experimental::optional<TLockedToken<CTexture>>& patternTex1,
                                     const std::experimental::optional<TLockedToken<CTexture>>& patternTex2,
                                     const std::experimental::optional<TLockedToken<CTexture>>& colorTex,
                                     const std::experimental::optional<TLockedToken<CTexture>>& bumpMap,
                                     const std::experimental::optional<TLockedToken<CTexture>>& envMap,
                                     const std::experimental::optional<TLockedToken<CTexture>>& envBumpMap,
                                     const std::experimental::optional<TLockedToken<CTexture>>& lightmap,
                                     bool doubleLightmapBlend, bool additive, u32 maxVertCount)
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
    boo::ObjToken<boo::IShaderPipeline> pipeline = _cache.GetOrBuildShader(shaderInfo);
    PrepareBinding(pipeline, maxVertCount, false);
}

CFluidPlaneShader::CFluidPlaneShader(const std::experimental::optional<TLockedToken<CTexture>>& patternTex1,
                                     const std::experimental::optional<TLockedToken<CTexture>>& patternTex2,
                                     const std::experimental::optional<TLockedToken<CTexture>>& colorTex,
                                     u32 maxVertCount)
: m_patternTex1(patternTex1),
  m_patternTex2(patternTex2),
  m_colorTex(colorTex)
{
    SFluidPlaneDoorShaderInfo shaderInfo(m_patternTex1.operator bool(),
                                         m_patternTex2.operator bool(),
                                         m_colorTex.operator bool());
    boo::ObjToken<boo::IShaderPipeline> pipeline = _cache.GetOrBuildShader(shaderInfo);
    PrepareBinding(pipeline, maxVertCount, true);
}

void CFluidPlaneShader::prepareDraw(const RenderSetupInfo& info)
{
    Uniform& uni = *reinterpret_cast<Uniform*>(m_uniBuf->map(sizeof(Uniform)));
    uni.m_mv = CGraphics::g_GXModelView.toMatrix4f();
    uni.m_mvNorm = info.normMtx;
    uni.m_proj = CGraphics::GetPerspectiveProjectionMatrix(true);
    for (int i=0 ; i<6 ; ++i)
        uni.m_texMtxs[i] = info.texMtxs[i];
    uni.m_lighting.ActivateLights(info.lights);
    for (int i=0 ; i<3 ; ++i)
        uni.m_lighting.colorRegs[i] = info.kColors[i];
    uni.m_lighting.mulColor = info.kColors[3];
    uni.m_lighting.fog.m_rangeScale = info.indScale;
    m_uniBuf->unmap();
    CGraphics::SetShaderDataBinding(m_dataBind);
}

void CFluidPlaneShader::loadVerts(const std::vector<Vertex>& verts)
{
    m_vbo->load(verts.data(), verts.size() * sizeof(Vertex));
}

}
