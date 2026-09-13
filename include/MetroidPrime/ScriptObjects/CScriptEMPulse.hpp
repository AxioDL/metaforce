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
  float xe8_duration;
  float xec_finalRadius;
  float xf0_currentRadius;
  float xf4_initialRadius;
  float xf8_interferenceDur;
  float xfc_;
  float x100_interferenceMag;
  float x104_;
  TLockedToken< CGenDescription > x108_particleDesc;
  rstl::single_ptr< CElementGen > x114_particleGen;

  CAABox CalculateBoundingBox() const;
};

#endif // _CSCRIPTEMPULSE
