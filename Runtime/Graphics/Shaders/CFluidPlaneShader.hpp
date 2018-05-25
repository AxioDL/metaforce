#ifndef CFLUIDPLANESHADER_HPP
#define CFLUIDPLANESHADER_HPP

#include "RetroTypes.hpp"
#include "boo/graphicsdev/IGraphicsDataFactory.hpp"
#include "Runtime/World/CFluidPlane.hpp"
#include "CModelShaders.hpp"
#include "boo/graphicsdev/GL.hpp"
#include "boo/graphicsdev/D3D.hpp"
#include "boo/graphicsdev/Metal.hpp"
#include "boo/graphicsdev/Vulkan.hpp"

namespace urde
{

struct SFluidPlaneShaderInfo
{
    CFluidPlane::EFluidType m_type;
    bool m_hasPatternTex1;
    bool m_hasPatternTex2;
    bool m_hasColorTex;
    bool m_hasBumpMap;
    bool m_hasEnvMap;
    bool m_hasEnvBumpMap;
    bool m_hasLightmap;
    bool m_doubleLightmapBlend;
    bool m_additive;

    SFluidPlaneShaderInfo(CFluidPlane::EFluidType type, bool hasPatternTex1, bool hasPatternTex2, bool hasColorTex,
                          bool hasBumpMap, bool hasEnvMap, bool hasEnvBumpMap, bool hasLightmap,
                          bool doubleLightmapBlend, bool additive)
    : m_type(type), m_hasPatternTex1(hasPatternTex1), m_hasPatternTex2(hasPatternTex2), m_hasColorTex(hasColorTex),
      m_hasBumpMap(hasBumpMap), m_hasEnvMap(hasEnvMap), m_hasEnvBumpMap(hasEnvBumpMap), m_hasLightmap(hasLightmap),
      m_doubleLightmapBlend(doubleLightmapBlend), m_additive(additive)
      {}
};

struct SFluidPlaneDoorShaderInfo
{
    bool m_hasPatternTex1;
    bool m_hasPatternTex2;
    bool m_hasColorTex;

    SFluidPlaneDoorShaderInfo(bool hasPatternTex1, bool hasPatternTex2, bool hasColorTex)
    : m_hasPatternTex1(hasPatternTex1), m_hasPatternTex2(hasPatternTex2), m_hasColorTex(hasColorTex)
    {}
};

class CFluidPlaneShader
{
public:
    struct Vertex
    {
        zeus::CVector3f m_pos;
        zeus::CVector3f m_norm;
        zeus::CVector3f m_binorm;
        zeus::CVector3f m_tangent;
        zeus::CColor m_color;

        Vertex() = default;
        Vertex(const zeus::CVector3f& position) : m_pos(position) {}
        Vertex(const zeus::CVector3f& position, const zeus::CColor& color)
        : m_pos(position), m_color(color) {}
        Vertex(const zeus::CVector3f& position, const zeus::CVector3f& normal,
               const zeus::CColor& color)
        : m_pos(position), m_norm(normal), m_color(color) {}
        Vertex(const zeus::CVector3f& position, const zeus::CVector3f& normal,
               const zeus::CVector3f& binormal, const zeus::CVector3f& tangent,
               const zeus::CColor& color)
        : m_pos(position), m_norm(normal), m_binorm(binormal), m_tangent(tangent), m_color(color) {}
    };

    struct RenderSetupInfo
    {
        zeus::CMatrix4f texMtxs[6];
        zeus::CMatrix4f normMtx;
        float indScale = 1.f;
        zeus::CColor kColors[4];
        std::vector<CLight> lights;
    };

private:
    class Cache
    {
        boo::ObjToken<boo::IShaderPipeline> m_cache[1024] = {};
        boo::ObjToken<boo::IShaderPipeline> m_doorCache[8] = {};
        boo::ObjToken<boo::IShaderPipeline>&
        CacheSlot(const SFluidPlaneShaderInfo& info, int i) { return m_cache[i]; }
        boo::ObjToken<boo::IShaderPipeline>&
        CacheSlot(const SFluidPlaneDoorShaderInfo& info, int i) { return m_doorCache[i]; }
        static u16 MakeCacheKey(const SFluidPlaneShaderInfo& info);
        static u16 MakeCacheKey(const SFluidPlaneDoorShaderInfo& info);
    public:
        template<class T>
        boo::ObjToken<boo::IShaderPipeline> GetOrBuildShader(const T& info);
        void Clear();
    };
    static Cache _cache;

    struct Uniform
    {
        zeus::CMatrix4f m_mv;
        zeus::CMatrix4f m_mvNorm;
        zeus::CMatrix4f m_proj;
        zeus::CMatrix4f m_texMtxs[9]; // Pad out to 768 bytes
        CModelShaders::LightingUniform m_lighting;
        zeus::CVector3f m_pad; // Pad out to 768 bytes
    };

    std::experimental::optional<TLockedToken<CTexture>> m_patternTex1;
    std::experimental::optional<TLockedToken<CTexture>> m_patternTex2;
    std::experimental::optional<TLockedToken<CTexture>> m_colorTex;
    std::experimental::optional<TLockedToken<CTexture>> m_bumpMap;
    std::experimental::optional<TLockedToken<CTexture>> m_envMap;
    std::experimental::optional<TLockedToken<CTexture>> m_envBumpMap;
    std::experimental::optional<TLockedToken<CTexture>> m_lightmap;
    boo::ObjToken<boo::IGraphicsBufferD> m_vbo;
    boo::ObjToken<boo::IGraphicsBufferD> m_uniBuf;
    boo::ObjToken<boo::IShaderDataBinding> m_dataBind;

#if BOO_HAS_GL
    static boo::ObjToken<boo::IShaderPipeline> BuildShader(boo::GLDataFactory::Context& ctx,
                                                           const SFluidPlaneShaderInfo& info);
    static boo::ObjToken<boo::IShaderPipeline> BuildShader(boo::GLDataFactory::Context& ctx,
                                                           const SFluidPlaneDoorShaderInfo& info);
    boo::ObjToken<boo::IShaderDataBinding> BuildBinding(boo::GLDataFactory::Context& ctx,
                                                        const boo::ObjToken<boo::IShaderPipeline>& pipeline, bool door);
#endif
#if _WIN32
    static boo::ObjToken<boo::IShaderPipeline> BuildShader(boo::D3DDataFactory::Context& ctx,
                                                           const SFluidPlaneShaderInfo& info);
    static boo::ObjToken<boo::IShaderPipeline> BuildShader(boo::D3DDataFactory::Context& ctx,
                                                           const SFluidPlaneDoorShaderInfo& info);
    boo::ObjToken<boo::IShaderDataBinding> BuildBinding(boo::D3DDataFactory::Context& ctx,
                                                        const boo::ObjToken<boo::IShaderPipeline>& pipeline, bool door);
#endif
#if BOO_HAS_METAL
    static boo::ObjToken<boo::IShaderPipeline> BuildShader(boo::MetalDataFactory::Context& ctx,
                                                           const SFluidPlaneShaderInfo& info);
    static boo::ObjToken<boo::IShaderPipeline> BuildShader(boo::MetalDataFactory::Context& ctx,
                                                           const SFluidPlaneDoorShaderInfo& info);
    boo::ObjToken<boo::IShaderDataBinding> BuildBinding(boo::MetalDataFactory::Context& ctx,
                                                        const boo::ObjToken<boo::IShaderPipeline>& pipeline, bool door);
#endif
#if BOO_HAS_VULKAN
    static boo::ObjToken<boo::IShaderPipeline> BuildShader(boo::VulkanDataFactory::Context& ctx,
                                                           const SFluidPlaneShaderInfo& info);
    static boo::ObjToken<boo::IShaderPipeline> BuildShader(boo::VulkanDataFactory::Context& ctx,
                                                           const SFluidPlaneDoorShaderInfo& info);
    boo::ObjToken<boo::IShaderDataBinding> BuildBinding(boo::VulkanDataFactory::Context& ctx,
                                                        const boo::ObjToken<boo::IShaderPipeline>& pipeline, bool door);
#endif

    template <class F> static void _Shutdown();

    void PrepareBinding(const boo::ObjToken<boo::IShaderPipeline>& pipeline, u32 maxVertCount, bool door);
public:
    CFluidPlaneShader(CFluidPlane::EFluidType type,
                      const std::experimental::optional<TLockedToken<CTexture>>& patternTex1,
                      const std::experimental::optional<TLockedToken<CTexture>>& patternTex2,
                      const std::experimental::optional<TLockedToken<CTexture>>& colorTex,
                      const std::experimental::optional<TLockedToken<CTexture>>& bumpMap,
                      const std::experimental::optional<TLockedToken<CTexture>>& envMap,
                      const std::experimental::optional<TLockedToken<CTexture>>& envBumpMap,
                      const std::experimental::optional<TLockedToken<CTexture>>& lightmap,
                      bool doubleLightmapBlend, bool additive, u32 maxVertCount);
    CFluidPlaneShader(const std::experimental::optional<TLockedToken<CTexture>>& patternTex1,
                      const std::experimental::optional<TLockedToken<CTexture>>& patternTex2,
                      const std::experimental::optional<TLockedToken<CTexture>>& colorTex,
                      u32 maxVertCount);
    void prepareDraw(const RenderSetupInfo& info);
    void loadVerts(const std::vector<Vertex>& verts);

    static void Shutdown();
};

}

#endif // CFLUIDPLANESHADER_HPP
