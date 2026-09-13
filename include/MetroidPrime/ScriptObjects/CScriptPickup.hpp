#ifndef _CSCRIPTPICKUP
#define _CSCRIPTPICKUP

#include "types.h"

#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"

class CGenDescription;

class CScriptPickup : public CPhysicsActor {
  CPlayerState::EItemType x258_itemType;
  int x25c_amount;
  int x260_capacity;
  float x264_possibility;
  float x268_fadeInTime;
  float x26c_lifeTime;
  float x270_curTime;
  float x274_tractorTime;
  float x278_delayTimer;
  rstl::optional_object< TCachedToken< CGenDescription > > x27c_pickupParticleDesc;
  bool x28c_24_generated : 1;
  bool x28c_25_inTractor : 1;
  bool x28c_26_enableTractorTest : 1;

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
