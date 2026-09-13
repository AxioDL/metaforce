#ifndef _CBOMB
#define _CBOMB

#include "MetroidPrime/Weapons/CWeapon.hpp"

#include <Kyoto/TToken.hpp>

class CElementGen;
class CGenDescription;
class CBomb : public CWeapon {
public:
  CBomb(TToken< CGenDescription > particle1, TToken< CGenDescription > particle2, TUniqueId uid,
        TAreaId aid, TUniqueId playerId, float f1, const CTransform4f& xf,
        const CDamageInfo& dInfo);
  ~CBomb();

  void Explode(const CVector3f& pos, CStateManager& mgr);
  void Touch(CActor& actor, CStateManager& mgr) override;
  void AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  void Think(float dt, CStateManager& mgr) override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void UpdateLight(float dt, CStateManager& mgr);

  void SetVelocityWR(const CVector3f& velocity) { mVelocity = velocity; }
  void SetConstantAccelerationWR(const CVector3f& acceleration) { mAcceleration = acceleration; }

  const bool IsDetonated() const { return !mIsNotDetonated; }

  const bool IsBeingDragged() const { return mBeingDragged; }

  void SetFuseDisabled(bool disabled) { mDisableFuse = disabled; }

  void SetIsBeingDragged(bool isBeingDragged) { mBeingDragged = isBeingDragged; }

private:
  CVector3f mVelocity;
  CVector3f mAcceleration;
  CVector3f mPrevLocation;
  float mFuseTime;
  rstl::single_ptr< CElementGen > mParticle1;
  rstl::single_ptr< CElementGen > mParticle2;
  TUniqueId mLightId;
  CAssetId mParticle2Ptr;
  bool mIsNotDetonated : 1;
  bool mBeingDragged : 1;
  bool mDisableFuse : 1;
};

#endif // _CBOMB
