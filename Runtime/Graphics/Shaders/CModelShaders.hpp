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

enum class EShaderType : uint8_t {
  DiffuseOnly,
  Normal,
  Dynamic,
  DynamicAlpha,
  DynamicCharacter
};

enum class EPostType : uint8_t {
  Normal,
  ThermalHot,
  ThermalCold,
  Solid,
  MBShadow,
  Disintegrate
};

class CModelShaders {
  friend class CModel;
  hsh::binding m_dataBind;

public:
  template <uint32_t NCol, uint32_t NUv, uint32_t NWeight>
  struct VertData {
    hsh::float3 posIn;
    hsh::float3 normIn;
    std::array<hsh::float4, NCol> colIn;
    std::array<hsh::float2, NUv> uvIn;
    std::array<hsh::float4, NWeight> weightIn;
  };

  template <uint32_t NSkinSlots>
  struct VertUniform {
    std::array<hsh::float4x4, NSkinSlots> objs;
    std::array<hsh::float4x4, NSkinSlots> objsInv;
    hsh::float4x4 mv;
    hsh::float4x4 mvInv;
    hsh::float4x4 proj;
  };

  struct TCGMatrix {
    hsh::float4x4 mtx;
    hsh::float4x4 postMtx;
  };

  struct ReflectMtx {
    hsh::float4x4 indMtx;
    hsh::float4x4 reflectMtx;
    float reflectAlpha;
  };

  struct Light {
    alignas(16) hsh::float3 pos;
    alignas(16) hsh::float3 dir;
    alignas(16) hsh::float4 color;
    alignas(16) hsh::float3 linAtt;
    alignas(16) hsh::float3 angAtt;
  };

  struct FragmentUniform {
    std::array<Light, URDE_MAX_LIGHTS> lights;
    hsh::float4 ambient;
    hsh::float4 lightmapMul;
    hsh::float4 flagsColor;
    CGraphics::CFogState fog;

    void ActivateLights(const std::vector<CLight>& lts);
  };

  hsh::binding& SetCurrent(const CModelFlags& modelFlags, const CBooSurface& surface, const CBooModel& model);

  using Material = DataSpec::DNAMP1::HMDLMaterialSet::Material;
};

} // namespace urde
