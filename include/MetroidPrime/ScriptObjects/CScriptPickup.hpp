#ifndef _CSCRIPTPICKUP
#define _CSCRIPTPICKUP

#include "types.h"

#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

class CGenDescription;

class CScriptPickup : public CPhysicsActor {
  CPlayerState::EItemType mItemType;
  int mAmount;
  int mCapacity;
  float mPossibility;
  float mFadeInTime;
  float mLifeTime;
  float mCurTime;
  float mTractorTime;
  float mDelayTimer;
  rstl::optional_object< TCachedToken< CGenDescription > > mPickupParticleDesc;
  bool mGenerated : 1;
  bool mInTractor : 1;
  bool mEnableTractorTest : 1;

public:
  CScriptPickup(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                const CTransform4f& xf, const CModelData& mData, const CActorParameters& aParams,
                const CAABox& aabb, CPlayerState::EItemType itemType, int amount, int capacity,
                CAssetId pickupEffect, float possibility, float lifeTime, float fadeInTime,
                float startDelay, const bool active);
  ~CScriptPickup();

  void Think(float, CStateManager&) override;
  void Touch(CActor&, CStateManager&) override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void Render(const CStateManager&) const override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;

  float GetPossibility() const;
  CPlayerState::EItemType GetItem() const;
  void SetWasGenerated();
};

#endif // _CSCRIPTPICKUP
