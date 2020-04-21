#pragma once

#include <string_view>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/World/CActor.hpp"

#include <zeus/COBBox.hpp>
#include <zeus/CRelAngle.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CScriptCameraPitchVolume : public CActor {
  static const zeus::CVector3f skScaleFactor;
  zeus::COBBox xe8_obbox;
  zeus::CRelAngle x124_upPitch;
  zeus::CRelAngle x128_downPitch;
  zeus::CVector3f x12c_scale;
  float x138_maxInterpDistance;
  bool x13c_24_entered : 1 = false;
  bool x13c_25_occupied : 1 = false;

public:
  CScriptCameraPitchVolume(TUniqueId, bool, std::string_view, const CEntityInfo&, const zeus::CVector3f&,
                           const zeus::CTransform&, const zeus::CRelAngle&, const zeus::CRelAngle&, float);

  void Accept(IVisitor& visitor) override;
  void Think(float, CStateManager&) override;
  std::optional<zeus::CAABox> GetTouchBounds() const override;
  void Touch(CActor&, CStateManager&) override;
  float GetUpPitch() const { return x124_upPitch; }
  float GetDownPitch() const { return x128_downPitch; }
  const zeus::CVector3f& GetScale() const { return x12c_scale; }
  float GetMaxInterpolationDistance() const { return x138_maxInterpDistance; }
  void Entered(CStateManager&);
  void Exited(CStateManager&);
};
} // namespace urde
