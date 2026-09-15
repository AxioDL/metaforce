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
  const CRelAngle& GetUpPitch() const { return x124_upPitch; }
  const CRelAngle& GetDownPitch() const { return x128_downPitch; }
  const CVector3f& GetScale() const { return x12c_scale; }
  float GetMaxInterpolationDistance() const { return x138_maxInterpDistance; }
  void Entered(CStateManager&);
  void Exited(CStateManager&);

private:
  static const CVector3f skScaleFactor;
  COBBox xe8_obbox;
  CRelAngle x124_upPitch;
  CRelAngle x128_downPitch;
  CVector3f x12c_scale;
  float x138_maxInterpDistance;
  bool x13c_24_entered : 1;
  bool x13c_25_occupied : 1;
};

#endif // _CSCRIPTCAMERAPITCHVOLUME
