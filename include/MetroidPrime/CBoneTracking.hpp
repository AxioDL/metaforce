#ifndef _CBONETRACKING
#define _CBONETRACKING

#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/string.hpp"

enum EBoneTrackingFlags {
  kBTF_None = 0,
  kBTF_NoParent = 1,
  kBTF_NoParentOrigin = 2,
  kBTF_NoHorizontalAim = 4,
  kBTF_ParentIk = 8,
};

class CAnimData;
class CStateManager;
class CTransform4f;
class CBodyController;

class CBoneTracking {
public:
  CBoneTracking(const CAnimData& animData, const rstl::string& bone, float maxTrackingAngle,
                float angSpeed, EBoneTrackingFlags flags);

  void Update(const float dt);
  void PreRender(const CStateManager& mgr, CAnimData& animData, const CTransform4f& xf,
                 const CVector3f& scale, const CBodyController& controller);
  void PreRender(const CStateManager& mgr, CAnimData& animData, const CTransform4f& xf,
                 const CVector3f& scale, const bool tracking);
  void SetActive(const bool v);
  void SetTarget(const TUniqueId uid);
  void UnsetTarget();
  void SetTargetPosition(const CVector3f& target);
  void SetNoHorizontalAim(const bool aim);

private:
  CQuaternion mRotation;
  float x10_;
  CSegId mSegId;
  float mTime;
  float mMaxTrackingAngle;
  float mAngSpeed;
  rstl::optional_object< CVector3f > mTargetPosition;
  TUniqueId mTarget;
  bool mActive : 1;
  bool mHasTrackedRotation : 1;
  bool mNoParent : 1;
  bool mNoParentOrigin : 1;
  bool mNoHorizontalAim : 1;
  bool mParentIk : 1;
};

CHECK_SIZEOF(CBoneTracking, 0x38)

#endif // _CBONETRACKING
