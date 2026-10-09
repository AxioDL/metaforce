#ifndef _CAIMINGCURSOR
#define _CAIMINGCURSOR

#include "types.h"

#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/TGameTypes.hpp"

class CFinalInput;
class CStateManager;

class CAimingCursor {
public:
  CAimingCursor(bool reservedFlag, uint reservedValue);

  CVector3f GetCursorOrbitPosition(const CStateManager& mgr) const;
  void UpdateAlpha(const CFinalInput& input, float dt, const CStateManager& mgr);
  void UpdateValidity(const CFinalInput& input, float dt, const CStateManager& mgr);
  bool CheckZeroCursorPosition(const CStateManager& mgr) const;
  void Update(const CFinalInput& input, float dt, CStateManager& mgr);

  CVector2f GetCursor2D() const;
  CVector3f GetCursorOnPlane() const;
  CVector3f GetCursorInWorld() const;
  TUniqueId GetCursorObjectId() const;
  uint GetCursorObjectCount() const;
  bool GetCursorValid() const;
  float GetCursorAlpha() const;
  CRayCastResult GetRaycastResult() const;
  bool IsHiddenForCSI() const;
  static float GetCursorPlaneDistance();
  bool ShowOffScreen(const CStateManager& mgr) const;

private:
  CVector2f mCursor2D;
  CVector3f mCursorOnPlane;
  CRayCastResult mRaycastResult;
  CVector3f mLastValidPointerPlane;
  CVector3f mCursorOrbitPosition;
  CVector3f mCursorVelocity;
  float mCursorVelocityMagnitude;
  CVector2f mCursorVelocity2D;
  float mCursorVelocity2DMagnitude;
  CVector3f mCursorInWorld;
  TUniqueId mCursorObjectId;
  uint mCursorObjectCount;
  float mCursorLockTimer;
  float mCursorAlpha;
  bool mCursorValid : 1;
  bool mReservedFlag : 1;
  CRelAngle mNunchukPitch;
  float mCursorFade;
  int mHideForCSICount;
  uint mReservedValue;
};
CHECK_SIZEOF(CAimingCursor, 0xb0)

#endif // _CAIMINGCURSOR
