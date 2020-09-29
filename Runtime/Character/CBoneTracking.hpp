#pragma once

#include <optional>
#include <string_view>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/Character/CSegId.hpp"

#include <zeus/CQuaternion.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CAnimData;
class CStateManager;
class CBodyController;

enum class EBoneTrackingFlags {
  None = 0,
  NoParent = 1,
  NoParentOrigin = 2,
  NoHorizontalAim = 4,
  ParentIk = 8
};
ENABLE_BITWISE_ENUM(EBoneTrackingFlags)

class CBoneTracking {
  zeus::CQuaternion x0_curRotation = zeus::CQuaternion();
  float x10_ = 0.f;
  CSegId x14_segId;
  float x18_time = 0.f;
  float x1c_maxTrackingAngle;
  float x20_angSpeed;
  std::optional<zeus::CVector3f> x24_targetPosition;
  TUniqueId x34_target = kInvalidUniqueId;
  bool x36_24_active : 1 = false;
  bool x36_25_hasTrackedRotation : 1 = false;
  bool x36_26_noParent : 1;
  bool x36_27_noParentOrigin : 1;
  bool x36_28_noHorizontalAim : 1;
  bool x36_29_parentIk : 1;

public:
  CBoneTracking(const CAnimData& animData, std::string_view bone,
                float maxTrackingAngle, float angSpeed, EBoneTrackingFlags flags);
  void Update(float dt);
  void PreRender(const CStateManager& mgr, CAnimData& animData, const zeus::CTransform& xf,
                 const zeus::CVector3f& vec, const CBodyController& bodyController);
  void PreRender(const CStateManager& mgr, CAnimData& animData, const zeus::CTransform& worldXf,
                 const zeus::CVector3f& localOffsetScale, bool tracking);
  void SetActive(bool active);
  void SetTarget(TUniqueId id);
  void UnsetTarget();
  void SetTargetPosition(const zeus::CVector3f& pos);
  void SetNoHorizontalAim(bool b);
};

} // namespace urde
