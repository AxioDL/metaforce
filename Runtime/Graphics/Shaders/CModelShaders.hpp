#pragma once

#include <array>
#include <cstdint>
#include <memory>

//#include "DataSpec/DNAMP1/CMDLMaterials.hpp"

#include "Runtime/Graphics/CGraphics.hpp"

#include <zeus/CColor.hpp>
#include <zeus/CVector3f.hpp>
#include <zeus/CVector4f.hpp>

#define URDE_MAX_LIGHTS 8

namespace hecl::Backend {
class ShaderTag;
}

namespace metaforce {
class CLight;

enum class EExtendedShader : uint8_t {
  Flat,
  Lighting,
  ThermalModel,
  ThermalModelNoZTestNoZWrite,
  ThermalStatic,
  ThermalStaticNoZWrite,
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

class CModelShaders {
  friend class CModel;

public:
//  struct Light {
//    zeus::CVector3f pos;
//    zeus::CVector3f dir;
//    zeus::CColor color = zeus::skClear;
//    std::array<float, 4> linAtt{1.f, 0.f, 0.f};
//    std::array<float, 4> angAtt{1.f, 0.f, 0.f};
//  };
//
//  struct LightingUniform {
//    std::array<Light, URDE_MAX_LIGHTS> lights;
//    zeus::CColor ambient;
//    std::array<zeus::CColor, 3> colorRegs;
//    zeus::CColor mulColor;
//    zeus::CColor addColor;
//    CFogState fog;
//
//    void ActivateLights(const std::vector<CLight>& lts);
//  };
//
//  struct ThermalUniform {
//    zeus::CColor mulColor;
//    zeus::CColor addColor;
//  };
//
//  struct SolidUniform {
//    zeus::CColor solidColor;
//  };
//
//  struct MBShadowUniform {
//    zeus::CVector4f shadowUp;
//    float shadowId;
//  };
//
//  struct OneTextureUniform {
//    zeus::CColor addColor;
//    CFogState fog;
//  };

  static void Initialize();
  static void Shutdown();

//  using ShaderPipelinesData = std::array<boo::ObjToken<boo::IShaderPipeline>, size_t(EExtendedShader::MAX)>;
//  using ShaderPipelines = std::shared_ptr<ShaderPipelinesData>;

//  using Material = DataSpec::DNAMP1::HMDLMaterialSet::Material;
//  static ShaderPipelines BuildExtendedShader(const hecl::Backend::ShaderTag& tag, const Material& material);

private:
//  static std::unordered_map<uint64_t, ShaderPipelines> g_ShaderPipelines;
};

} // namespace metaforce
