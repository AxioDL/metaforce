#pragma once

#include <string>

#include "Runtime/IOStreams.hpp"
#include "Runtime/RetroTypes.hpp"

#include <zeus/CVector3f.hpp>

namespace urde {

class CParticleData {
public:
  enum class EParentedMode { Initial, ContinuousEmitter, ContinuousSystem };

private:
  u32 x0_duration = 0;
  SObjectTag x4_particle;
  std::string xc_boneName = "root";
  float x1c_scale = 1.f;
  EParentedMode x20_parentMode = EParentedMode::Initial;

public:
  CParticleData() = default;
  CParticleData(CInputStream& in);
  u32 GetDuration() const { return x0_duration; }
  const SObjectTag& GetTag() const { return x4_particle; }
  std::string_view GetSegmentName() const { return xc_boneName; }
  float GetScale() const { return x1c_scale; }
  EParentedMode GetParentedMode() const { return x20_parentMode; }
};

class CAuxiliaryParticleData {
  u32 x0_duration = 0;
  SObjectTag x4_particle;
  zeus::CVector3f xc_translation;
  float x18_scale = 1.f;

public:
  u32 GetDuration() const { return x0_duration; }
  const SObjectTag& GetTag() const { return x4_particle; }
  const zeus::CVector3f& GetTranslation() const { return xc_translation; }
  float GetScale() const { return x18_scale; }
};

} // namespace urde
