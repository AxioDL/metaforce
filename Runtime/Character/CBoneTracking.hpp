#pragma once

#include "Character/CSegId.hpp"

#include "zeus/CTransform.hpp"
#include "zeus/CQuaternion.hpp"
#include "zeus/CVector3f.hpp"
#include "RetroTypes.hpp"

namespace urde {
class CAnimData;
class CStateManager;
class CBodyController;
class CBoneTracking {
  zeus::CQuaternion x0_ = zeus::CQuaternion::skNoRotation;
  CSegId x14_segId;
  float x18_time = 0.f;
  float x1c_;
  float x20_;
  std::experimental::optional<zeus::CVector3f> x24_targetPosition;
  TUniqueId x34_target = kInvalidUniqueId;
  bool x36_24_active : 1;
  bool x36_25_ : 1;
  bool x36_26_ : 1;
  bool x36_27_ : 1;
  bool x36_28_ : 1;
  bool x36_29_ : 1;

public:
  CBoneTracking(const CAnimData&, std::string_view, float, float, bool);
  void Update(float dt);
  void PreRender(const CStateManager&, CAnimData&, const zeus::CTransform&, const zeus::CVector3f&,
                 const CBodyController&);
  void PreRender(const CStateManager&, CAnimData&, const zeus::CTransform&, const zeus::CVector3f&, bool);
  void SetActive(bool);
  void SetTarget(TUniqueId);
  void SetTargetPosition(const zeus::CVector3f&);
};
} // namespace urde