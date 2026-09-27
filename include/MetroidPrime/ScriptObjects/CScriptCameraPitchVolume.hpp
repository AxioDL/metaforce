#ifndef _CSCRIPTCAMERAPITCHVOLUME
#define _CSCRIPTCAMERAPITCHVOLUME

#include "MetroidPrime/CActor.hpp"

#include "Collision/COBBox.hpp"
#include "Kyoto/Math/CRelAngle.hpp"

class CScriptCameraPitchVolume : public CActor {
public:
  CScriptCameraPitchVolume(TUniqueId, bool, const rstl::string&, const CEntityInfo&,
                           const CVector3f&, const CTransform4f&, const CRelAngle&,
                           const CRelAngle&, float);

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float, CStateManager&) override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor&, CStateManager&) override;
  const CRelAngle& GetUpPitch() const { return mUpPitch; }
  const CRelAngle& GetDownPitch() const { return mDownPitch; }
  const CVector3f& GetScale() const { return mScale; }
  float GetMaxInterpolationDistance() const { return mMaxInterpDistance; }
  void Entered(CStateManager&);
  void Exited(CStateManager&);

private:
  static const CVector3f skScaleFactor;
  COBBox mObbox;
  CRelAngle mUpPitch;
  CRelAngle mDownPitch;
  CVector3f mScale;
  float mMaxInterpDistance;
  bool mEntered : 1;
  bool mOccupied : 1;
};

#endif // _CSCRIPTCAMERAPITCHVOLUME
