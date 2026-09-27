#ifndef _CSCRIPTEMPULSE
#define _CSCRIPTEMPULSE

#include "MetroidPrime/CActor.hpp"

class CGenDescription;
class CElementGen;

class CScriptEMPulse : public CActor {
public:
  CScriptEMPulse(TUniqueId, const rstl::string&, const CEntityInfo&, const CTransform4f&, bool,
                 float, float, float, float, float, float, float, CAssetId);

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float, CStateManager&) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void AddToRenderer(const CFrustumPlanes&, const CStateManager&) const override;
  void CalculateRenderBounds() override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor&, CStateManager&) override;

private:
  float mDuration;
  float mFinalRadius;
  float mCurrentRadius;
  float mInitialRadius;
  float mInterferenceDur;
  float xfc_;
  float mInterferenceMag;
  float x104_;
  TLockedToken< CGenDescription > mParticleDesc;
  rstl::single_ptr< CElementGen > mParticleGen;

  CAABox CalculateBoundingBox() const;
};

#endif // _CSCRIPTEMPULSE
