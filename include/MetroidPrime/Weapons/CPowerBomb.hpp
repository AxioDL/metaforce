#ifndef _CPOWERBOMB
#define _CPOWERBOMB

#include "types.h"

#include "MetroidPrime/Weapons/CWeapon.hpp"

class CGenDescription;
class CElementGen;

class CPowerBomb : public CWeapon {
public:
  CPowerBomb(TToken< CGenDescription > particle, TUniqueId uid, TAreaId aid,
             TUniqueId playerId, const CTransform4f& xf, const CDamageInfo& dInfo);
  ~CPowerBomb();

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void Think(float, CStateManager&) override;
  void AddToRenderer(const CFrustumPlanes&, const CStateManager&) const override;
  void Render(const CStateManager&) const override;
  rstl::optional_object<CAABox> GetTouchBounds() const override;
  void Touch(CActor&, CStateManager&) override;

  float GetCurTime() const { return mCurTime; }
  bool IsEnding() const { return mCurTime > kEndingTime; }
  void ApplyDynamicDamage(const CVector3f&, CStateManager&);

  static const CColor& FadeColor() { return kFadeColor; }
  static const float EndingTime() { return kEndingTime; }
private:
  static CColor kFadeColor;
  static const float kEndingTime;

  bool mCanStartFilter : 1;
  bool mFilterEnabled : 1;
  float mCurTime;
  float mCurRadius;
  float mRadiusIncrement;
  rstl::single_ptr< CElementGen > mParticle;
  float mRadius;
};

#endif // _CPOWERBOMB
