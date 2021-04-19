#pragma once

#include <array>
#include <vector>

#include "Runtime/CToken.hpp"
#include "Runtime/Graphics/CTexture.hpp"
#include "Runtime/Graphics/Shaders/CModelShaders.hpp"
#include "Runtime/World/CFluidPlaneManager.hpp"

#include "zeus/CColor.hpp"
#include "zeus/CMatrix4f.hpp"
#include "zeus/CVector3f.hpp"
#include "zeus/CVector4f.hpp"

namespace metaforce {

enum class EFluidType { NormalWater, PoisonWater, Lava, PhazonFluid, Four, ThickLava };

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
  struct BindingPair {
    hsh::binding m_regular, m_tessellation;
  };

  struct Ripple {
    zeus::CVector4f center; // time, distFalloff
    zeus::CVector4f params; // amplitude, lookupPhase, lookupTime
  };

  struct Uniform {
    zeus::CMatrix4f m_mv;
    zeus::CMatrix4f m_mvNorm;
    zeus::CMatrix4f m_proj;
    std::array<zeus::CMatrix4f, 6> m_texMtxs;
    std::array<Ripple, 20> m_ripple;
    zeus::CVector4f m_colorMul;
    std::array<zeus::CVector4f, 3> m_pad; // rippleNormResolution, Pad out to 1280 bytes
    CModelShaders::LightingUniform m_lighting;
    zeus::CVector3f m_pad2; // Pad out to 768 bytes, also holds ind scale
  };

  TLockedToken<CTexture> m_patternTex1;
  TLockedToken<CTexture> m_patternTex2;
  TLockedToken<CTexture> m_colorTex;
  TLockedToken<CTexture> m_bumpMap;
  TLockedToken<CTexture> m_envMap;
  TLockedToken<CTexture> m_envBumpMap;
  TLockedToken<CTexture> m_lightmap;
  hsh::texture2d m_rippleMap;
  hsh::dynamic_owner<hsh::vertex_buffer<Vertex>> m_vbo;
  hsh::dynamic_owner<hsh::vertex_buffer<PatchVertex>> m_pvbo;
  hsh::dynamic_owner<hsh::uniform_buffer<Uniform>> m_uniBuf;
  BindingPair m_dataBind;

  void PrepareBinding(u32 maxVertCount);

public:
  CFluidPlaneShader(EFluidType type, const TLockedToken<CTexture>& patternTex1,
                    const TLockedToken<CTexture>& patternTex2, const TLockedToken<CTexture>& colorTex,
                    const TLockedToken<CTexture>& bumpMap, const TLockedToken<CTexture>& envMap,
                    const TLockedToken<CTexture>& envBumpMap, const TLockedToken<CTexture>& lightmap,
                    hsh::texture2d rippleMap, bool doubleLightmapBlend, bool additive,
                    u32 maxVertCount);
  CFluidPlaneShader(const TLockedToken<CTexture>& patternTex1, const TLockedToken<CTexture>& patternTex2,
                    const TLockedToken<CTexture>& colorTex, u32 maxVertCount);
  void prepareDraw(const RenderSetupInfo& info);
  void prepareDraw(const RenderSetupInfo& info, const zeus::CVector3f& waterCenter, const CRippleManager& rippleManager,
                   const zeus::CColor& colorMul, float rippleNormResolution);
  void drawRegular(u32 start, u32 count) {
    m_dataBind.m_regular.draw(start, count);
  }
  void drawTessellation(u32 start, u32 count) {
    m_dataBind.m_tessellation.draw(start, count);
  }
  void loadVerts(const std::vector<Vertex>& verts, const std::vector<PatchVertex>& pVerts);
};

} // namespace metaforce
