#pragma once

#include <array>
#include <cstdint>
#include <memory>

#include "DataSpec/DNAMP1/CMDLMaterials.hpp"

#include "Runtime/Graphics/CGraphics.hpp"

#include <zeus/CColor.hpp>
#include <zeus/CVector3f.hpp>
#include <zeus/CVector4f.hpp>

#define URDE_MAX_LIGHTS 8

namespace hecl::Backend {
class ShaderTag;
} // namespace hecl::Backend

namespace urde {
class CLight;
struct CModelFlags;
struct CBooSurface;
class CBooModel;

enum class EExtendedShader : uint8_t {
  Flat,
  Lighting,
  Thermal,
  ForcedAlpha,
  ForcedAdditive,
  SolidColor,
  SolidColorAdditive,
  SolidColorFrontfaceCullLEqualAlphaOnly,
  SolidColorFrontfaceCullAlwaysAlphaOnly, // No Z-write or test
  SolidColorBackfaceCullLEqualAlphaOnly,
  SolidColorBackfaceCullGreaterAlphaOnly, // No Z-write
  MorphBallShadow,
  WorldShadow,
  ForcedAlphaNoCull,
  ForcedAdditiveNoCull,
  ForcedAlphaNoZWrite,
  ForcedAdditiveNoZWrite,
  ForcedAlphaNoCullNoZWrite,
  ForcedAdditiveNoCullNoZWrite,
  DepthGEqualNoZWrite,
  Disintegrate,
  ForcedAdditiveNoZWriteDepthGreater,
  ThermalCold,
  LightingAlphaWrite,
  LightingAlphaWriteNoZTestNoZWrite,
  LightingCubeReflection,
  LightingCubeReflectionWorldShadow,
  MAX
};

enum class EShaderType : uint8_t { DiffuseOnly, Normal, Dynamic, DynamicAlpha, DynamicCharacter };

enum class EPostType : uint8_t { Normal, ThermalHot, ThermalCold, Solid, MBShadow, Disintegrate };

struct ModelInstance;

class CModelShaders {
  friend class CModel;

public:
  template <uint32_t NCol, uint32_t NUv, uint32_t NWeight>
  struct VertData {
    hsh::float3 posIn;
    hsh::float3 normIn;
    // FIXME: compiler bug?
//  [[no_unique_address]] hsh::array<hsh::float4, NCol> colIn;
    [[no_unique_address]] hsh::array<hsh::float2, NUv> uvIn;
    [[no_unique_address]] hsh::array<hsh::float4, NWeight> weightIn;
  };
  static_assert(sizeof(VertData<0, 0, 0>) == 24, "VertData size incorrect");
  static_assert(sizeof(VertData<0, 1, 0>) == 32, "VertData size incorrect");
  static_assert(sizeof(VertData<0, 2, 0>) == 40, "VertData size incorrect");
  static_assert(sizeof(VertData<0, 1, 1>) == 48, "VertData size incorrect");

  template <uint32_t NSkinSlots>
  struct VertUniform {
    hsh::array<hsh::float4x4, NSkinSlots> objs [[no_unique_address]];
    hsh::array<hsh::float4x4, NSkinSlots> objsInv [[no_unique_address]];
    hsh::float4x4 mv;
    hsh::float4x4 mvInv;
    hsh::float4x4 proj;
  };
  static_assert(sizeof(VertUniform<0>) == 192, "VertUniform size incorrect");
  static_assert(sizeof(VertUniform<1>) == 320, "VertUniform size incorrect");

  struct TCGMatrix {
    hsh::float4x4 mtx;
    hsh::float4x4 postMtx;
  };
  using TCGMatrixUniform = std::array<CModelShaders::TCGMatrix, 8>;

  struct ReflectMtx {
    hsh::float4x4 indMtx;
    hsh::float4x4 reflectMtx;
    float reflectAlpha;
  };

  struct Light {
    alignas(16) hsh::float3 pos;
    alignas(16) hsh::float3 dir;
    alignas(16) hsh::float4 color;
    alignas(16) hsh::float3 linAtt{1.f, 0.f, 0.f};
    alignas(16) hsh::float3 angAtt{1.f, 0.f, 0.f};
  };

  struct FragmentUniform {
    std::array<Light, URDE_MAX_LIGHTS> lights;
    hsh::float4 ambient;
    hsh::float4 lightmapMul;
    hsh::float4 flagsColor;
    CGraphics::CFogState fog;

    void ActivateLights(const std::vector<CLight>& lts);
  };

  static void SetCurrent(hsh::binding& binding, const CModelFlags& modelFlags, const CBooModel& model,
                         const ModelInstance& inst, const CBooSurface& surface);

  using Material = DataSpec::DNAMP1::HMDLMaterialSet::Material;
};

struct ModelInstance {
  hsh::dynamic_owner<hsh::uniform_buffer<CModelShaders::FragmentUniform>> m_fragmentUniform;
  std::vector<hsh::dynamic_owner<hsh::uniform_buffer<CModelShaders::TCGMatrixUniform>>> m_tcgUniforms;
  std::vector<hsh::dynamic_owner<hsh::uniform_buffer<CModelShaders::ReflectMtx>>> m_reflectUniforms;
  std::vector<hsh::dynamic_owner<hsh::uniform_buffer_typeless>> m_geometryUniforms;
  std::vector<hsh::binding> m_shaderDataBindings;
  mutable hsh::dynamic_owner<hsh::vertex_buffer_typeless> m_dynamicVbo;

  hsh::vertex_buffer_typeless GetBooVBO(const CBooModel& model) const;
};


} // namespace urde
