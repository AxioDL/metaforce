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

private:
    class Cache
    {
        std::pair<boo::GraphicsDataToken, boo::IShaderPipeline*> m_cache[1024] = {};
        static u16 MakeCacheKey(const SFluidPlaneShaderInfo& info);
    public:
        boo::IShaderPipeline* GetOrBuildShader(const SFluidPlaneShaderInfo& info);
    };
    static Cache _cache;

    struct Uniform
    {
        zeus::CMatrix4f m_mv;
        zeus::CMatrix4f m_mvNorm;
        zeus::CMatrix4f m_proj;
        zeus::CMatrix4f m_texMtxs[9]; // Pad out to 768 bytes
        CModelShaders::LightingUniform m_lighting;
    };

    std::experimental::optional<TLockedToken<CTexture>> m_patternTex1;
    std::experimental::optional<TLockedToken<CTexture>> m_patternTex2;
    std::experimental::optional<TLockedToken<CTexture>> m_colorTex;
    std::experimental::optional<TLockedToken<CTexture>> m_bumpMap;
    std::experimental::optional<TLockedToken<CTexture>> m_envMap;
    std::experimental::optional<TLockedToken<CTexture>> m_envBumpMap;
    std::experimental::optional<TLockedToken<CTexture>> m_lightmap;
    boo::GraphicsDataToken m_gfxTok;
    boo::IGraphicsBufferD* m_vbo;
    boo::IGraphicsBufferD* m_uniBuf;
    boo::IShaderDataBinding* m_dataBind;

    static boo::IShaderPipeline* BuildShader(boo::GLDataFactory::Context& ctx, const SFluidPlaneShaderInfo& info);
    boo::IShaderDataBinding* BuildBinding(boo::GLDataFactory::Context& ctx, boo::IShaderPipeline* pipeline);
#if _WIN32
    static boo::IShaderPipeline* BuildShader(boo::ID3DDataFactory::Context& ctx, const SFluidPlaneShaderInfo& info);
    boo::IShaderDataBinding* BuildBinding(boo::ID3DDataFactory::Context& ctx, boo::IShaderPipeline* pipeline);
#endif
#if BOO_HAS_METAL
    static boo::IShaderPipeline* BuildShader(boo::MetalDataFactory::Context& ctx, const SFluidPlaneShaderInfo& info);
    boo::IShaderDataBinding* BuildBinding(boo::MetalDataFactory::Context& ctx, boo::IShaderPipeline* pipeline);
#endif
#if BOO_HAS_VULKAN
    static boo::IShaderPipeline* BuildShader(boo::VulkanDataFactory::Context& ctx, const SFluidPlaneShaderInfo& info);
    boo::IShaderDataBinding* BuildBinding(boo::VulkanDataFactory::Context& ctx, boo::IShaderPipeline* pipeline);
#endif

public:
    CFluidPlaneShader(CFluidPlane::EFluidType type,
                      const std::experimental::optional<TLockedToken<CTexture>>& patternTex1,
                      const std::experimental::optional<TLockedToken<CTexture>>& patternTex2,
                      const std::experimental::optional<TLockedToken<CTexture>>& colorTex,
                      const std::experimental::optional<TLockedToken<CTexture>>& bumpMap,
                      const std::experimental::optional<TLockedToken<CTexture>>& envMap,
                      const std::experimental::optional<TLockedToken<CTexture>>& envBumpMap,
                      const std::experimental::optional<TLockedToken<CTexture>>& lightmap,
                      bool doubleLightmapBlend, bool additive);
    void prepareDraw(const zeus::CMatrix4f* texMtxs, const zeus::CMatrix4f& normMtx, float indScale,
                     const std::vector<CLight>& lights, const zeus::CColor* kColors);
    void loadVerts(const std::vector<Vertex>& verts);
};

}

#endif // CFLUIDPLANESHADER_HPP
