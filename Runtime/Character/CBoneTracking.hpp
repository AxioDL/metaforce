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
  zeus::CQuaternion x0_curRotation = zeus::CQuaternion::skNoRotation;
  float x10_ = 0.f;
  CSegId x14_segId;
  float x18_time = 0.f;
  float x1c_maxTrackingAngle;
  float x20_angSpeed;
  rstl::optional<zeus::CVector3f> x24_targetPosition;
  TUniqueId x34_target = kInvalidUniqueId;
  union {
    struct {
      bool x36_24_active : 1;
      bool x36_25_hasTrackedRotation : 1;
      bool x36_26_noParent : 1;
      bool x36_27_noParentOrigin : 1;
      bool x36_28_noHorizontalAim : 1;
      bool x36_29_parentIk : 1;
    };
    u32 _dummy = 0;
  };

public:
  CBoneTracking(const CAnimData& animData, std::string_view bone,
                float maxTrackingAngle, float angSpeed, bool parentIk);
  void Update(float dt);
  void PreRender(const CStateManager& mgr, CAnimData& animData, const zeus::CTransform& xf,
                 const zeus::CVector3f& vec, const CBodyController& bodyController);
  void PreRender(const CStateManager& mgr, CAnimData& animData, const zeus::CTransform& worldXf,
                 const zeus::CVector3f& localOffsetScale, bool tracking);
  void SetActive(bool b);
  void SetTarget(TUniqueId id);
  void UnsetTarget();
  void SetTargetPosition(const zeus::CVector3f& pos);
  void SetNoHorizontalAim(bool b);
};
} // namespace urde