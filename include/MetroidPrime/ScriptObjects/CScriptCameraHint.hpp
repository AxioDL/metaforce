#ifndef _CSCRIPTCAMERAHINT
#define _CSCRIPTCAMERAHINT

#include "MetroidPrime/CActor.hpp"
#include "MetroidPrime/Cameras/CBallCamera.hpp"
#include "MetroidPrime/Cameras/CCameraOverrideInfo.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Math/CTransform4f.hpp"

#include "rstl/reserved_vector.hpp"

class CCameraSpring;

class CScriptCameraHint : public CActor {
public:
  CScriptCameraHint(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                    const CTransform4f& xf, bool active, int priority,
                    CBallCamera::EBallCameraBehaviour behavior, int overrideFlags, float minDist,
                    float maxDist, float backwardsDist, const CVector3f& lookAtOffset,
                    const CVector3f& chaseLookAtOffset, const CVector3f& ballToCam, float fov,
                    float attitudeRange, float azimuthRange, float anglePerSecond,
                    float clampVelRange, float clampRotRange, float elevation,
                    float interpolateTime, float clampVelTime, float controlInterpDur);

  // CEntity
  ~CScriptCameraHint() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  TUniqueId GetSenderId() const;
  void ClearSenders();
  void AddSender(TUniqueId uid);
  void RemoveSender(TUniqueId uid, CStateManager& mgr);
  void CheckLegacyConnections(CStateManager& mgr);

  const CCameraOverrideInfo& GetInfo() const { return mOverrideInfo; } // Not real?
  uint GetOverrideFlags() const { return mOverrideInfo.mOverrideFlags; }
  CBallCamera::EBallCameraBehaviour GetBehaviourType() const { return mOverrideInfo.mBehaviour; }
  int GetPriority() const { return mPriority; }
  TUniqueId GetDelegatedCameraId() const { return mDelegatedCameraId; }
  const CTransform4f& GetOriginalTransform() const { return mOrigXf; }
  // OverrideCameraInfo__17CScriptCameraHintFP11CBallCamera

  uint GetSenderCount() const { return mSenders.size(); }
  bool GetInactive() const { return mInactive; }
  void SetInactive(bool inactive) { mInactive = inactive; }

private:
  int mPriority;
  CCameraOverrideInfo mOverrideInfo;
  rstl::reserved_vector< TUniqueId, 8 > mSenders;
  TUniqueId mDelegatedCameraId;
  bool mInactive;
  CTransform4f mOrigXf;
};
CHECK_SIZEOF(CScriptCameraHint, (VERSION >= VERSION_GM8E_02 ? 0x1a8 : 0x198))

#endif // _CSCRIPTCAMERAHINT
