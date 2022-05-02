#pragma once

#include <array>
#include <vector>

#include "Runtime/CToken.hpp"
#include "Runtime/Graphics/CTexture.hpp"
#include "Runtime/Graphics/Shaders/CModelShaders.hpp"
#include "Runtime/World/CFluidPlaneManager.hpp"

#include "Shaders/shader_CFluidPlaneShader.hpp"

//#include <boo/graphicsdev/IGraphicsDataFactory.hpp>

#include <zeus/CColor.hpp>
#include <zeus/CMatrix4f.hpp>
#include <zeus/CVector3f.hpp>
#include <zeus/CVector4f.hpp>

namespace metaforce {

class CFluidPlaneShader {
public:
  struct Vertex {
    zeus::CVector3f m_pos;
    zeus::CVector3f m_norm;
    zeus::CVector3f m_binorm;
    zeus::CVector3f m_tangent;
    zeus::CColor m_color;

    Vertex() = default;
    Vertex(const zeus::CVector3f& position) : m_pos(position) {}
    Vertex(const zeus::CVector3f& position, const zeus::CColor& color) : m_pos(position), m_color(color) {}
    Vertex(const zeus::CVector3f& position, const zeus::CVector3f& normal, const zeus::CColor& color)
    : m_pos(position), m_norm(normal), m_color(color) {}
    Vertex(const zeus::CVector3f& position, const zeus::CVector3f& normal, const zeus::CVector3f& binormal,
           const zeus::CVector3f& tangent, const zeus::CColor& color)
    : m_pos(position), m_norm(normal), m_binorm(binormal), m_tangent(tangent), m_color(color) {}
  };

  struct PatchVertex {
    zeus::CVector4f m_pos;
    std::array<float, 4> m_outerLevels{};
    std::array<float, 4> m_innerLevels{};
  };

  struct RenderSetupInfo {
    std::array<zeus::CMatrix4f, 6> texMtxs;
    zeus::CMatrix4f normMtx;
    float indScale = 1.f;
    std::array<zeus::CColor, 4> kColors;
    std::vector<CLight> lights;
  };

private:
//  struct ShaderPair {
//    boo::ObjToken<boo::IShaderPipeline> m_regular;
//    boo::ObjToken<boo::IShaderPipeline> m_tessellation;
//    void reset() {
//      m_regular.reset();
//      m_tessellation.reset();
//    }
//  };
//
//  struct BindingPair {
//    boo::ObjToken<boo::IShaderDataBinding> m_regular;
//    boo::ObjToken<boo::IShaderDataBinding> m_tessellation;
//  };

//  class Cache {
//    std::array<ShaderPair, 1024> m_cache{};
//    std::array<ShaderPair, 8> m_doorCache{};
//    ShaderPair& CacheSlot(const SFluidPlaneShaderInfo& info, int i) { return m_cache[i]; }
//    ShaderPair& CacheSlot(const SFluidPlaneDoorShaderInfo& info, int i) { return m_doorCache[i]; }
//    static u16 MakeCacheKey(const SFluidPlaneShaderInfo& info);
//    static u16 MakeCacheKey(const SFluidPlaneDoorShaderInfo& info);
//
//  public:
//    template <class T>
//    ShaderPair GetOrBuildShader(const T& info);
//    void Clear();
//  };
//  static Cache _cache;

  struct Ripple {
    zeus::CVector4f center; // time, distFalloff
    zeus::CVector4f params; // amplitude, lookupPhase, lookupTime
  };

//  struct Uniform {
//    zeus::CMatrix4f m_mv;
//    zeus::CMatrix4f m_mvNorm;
//    zeus::CMatrix4f m_proj;
//    std::array<zeus::CMatrix4f, 6> m_texMtxs;
//    std::array<Ripple, 20> m_ripple;
//    zeus::CVector4f m_colorMul;
//    std::array<zeus::CVector4f, 3> m_pad; // rippleNormResolution, Pad out to 1280 bytes
//    CModelShaders::LightingUniform m_lighting;
//    zeus::CVector3f m_pad2; // Pad out to 768 bytes, also holds ind scale
//  };

  TLockedToken<CTexture> m_patternTex1;
  TLockedToken<CTexture> m_patternTex2;
  TLockedToken<CTexture> m_colorTex;
  TLockedToken<CTexture> m_bumpMap;
  TLockedToken<CTexture> m_envMap;
  TLockedToken<CTexture> m_envBumpMap;
  TLockedToken<CTexture> m_lightmap;
  aurora::gfx::TextureHandle m_rippleMap;
//  boo::ObjToken<boo::IGraphicsBufferD> m_vbo;
//  boo::ObjToken<boo::IGraphicsBufferD> m_pvbo;
//  boo::ObjToken<boo::IGraphicsBufferD> m_uniBuf;
//  ShaderPair m_pipelines;
//  BindingPair m_dataBind;
  int m_lastBind = -1;

//#if BOO_HAS_GL
//  static ShaderPair BuildShader(boo::GLDataFactory::Context& ctx, const SFluidPlaneShaderInfo& info);
//  static ShaderPair BuildShader(boo::GLDataFactory::Context& ctx, const SFluidPlaneDoorShaderInfo& info);
//  BindingPair BuildBinding(boo::GLDataFactory::Context& ctx, const ShaderPair& pipeline);
//#endif
//#if _WIN32
//  static ShaderPair BuildShader(boo::D3D11DataFactory::Context& ctx, const SFluidPlaneShaderInfo& info);
//  static ShaderPair BuildShader(boo::D3D11DataFactory::Context& ctx, const SFluidPlaneDoorShaderInfo& info);
//  BindingPair BuildBinding(boo::D3D11DataFactory::Context& ctx, const ShaderPair& pipeline);
//#endif
//#if BOO_HAS_METAL
//  static ShaderPair BuildShader(boo::MetalDataFactory::Context& ctx, const SFluidPlaneShaderInfo& info);
//  static ShaderPair BuildShader(boo::MetalDataFactory::Context& ctx, const SFluidPlaneDoorShaderInfo& info);
//  BindingPair BuildBinding(boo::MetalDataFactory::Context& ctx, const ShaderPair& pipeline);
//#endif
//#if BOO_HAS_VULKAN
//  static ShaderPair BuildShader(boo::VulkanDataFactory::Context& ctx, const SFluidPlaneShaderInfo& info);
//  static ShaderPair BuildShader(boo::VulkanDataFactory::Context& ctx, const SFluidPlaneDoorShaderInfo& info);
//  BindingPair BuildBinding(boo::VulkanDataFactory::Context& ctx, const ShaderPair& pipeline);
//#endif

  template <class F>
  static void _Shutdown();

  void PrepareBinding(u32 maxVertCount);

public:
  CFluidPlaneShader(EFluidType type, const TLockedToken<CTexture>& patternTex1,
                    const TLockedToken<CTexture>& patternTex2, const TLockedToken<CTexture>& colorTex,
                    const TLockedToken<CTexture>& bumpMap, const TLockedToken<CTexture>& envMap,
                    const TLockedToken<CTexture>& envBumpMap, const TLockedToken<CTexture>& lightmap,
                    const aurora::gfx::TextureHandle& rippleMap, bool doubleLightmapBlend, bool additive,
                    u32 maxVertCount);
  CFluidPlaneShader(const TLockedToken<CTexture>& patternTex1, const TLockedToken<CTexture>& patternTex2,
                    const TLockedToken<CTexture>& colorTex, u32 maxVertCount);
  void prepareDraw(const RenderSetupInfo& info);
  void prepareDraw(const RenderSetupInfo& info, const zeus::CVector3f& waterCenter, const CRippleManager& rippleManager,
                   const zeus::CColor& colorMul, float rippleNormResolution);
  void bindRegular() {
    if (m_lastBind != 0) {
//      CGraphics::SetShaderDataBinding(m_dataBind.m_regular);
      m_lastBind = 0;
    }
  }
  bool bindTessellation() {
    if (m_lastBind != 1) {
//      CGraphics::SetShaderDataBinding(m_dataBind.m_tessellation);
      m_lastBind = 1;
    }
    return true;
  }
  void doneDrawing() { m_lastBind = -1; }
  void loadVerts(const std::vector<Vertex>& verts, const std::vector<PatchVertex>& pVerts);
  bool isReady() const {
//    return m_pipelines.m_regular->isReady() && (!m_pipelines.m_tessellation || m_pipelines.m_tessellation->isReady());
    return false;
  }

  static void Shutdown();
};

} // namespace metaforce
