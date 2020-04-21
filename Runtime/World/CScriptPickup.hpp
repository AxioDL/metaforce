#pragma once

#include <string_view>

#include "Runtime/CPlayerState.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/World/CPhysicsActor.hpp"

namespace urde {
class CScriptPickup : public CPhysicsActor {
  CPlayerState::EItemType x258_itemType;
  s32 x25c_amount;
  s32 x260_capacity;
  float x264_possibility;
  float x268_fadeInTime;
  float x26c_lifeTime;
  float x270_curTime = 0.f;
  float x274_tractorTime = 0.f;
  float x278_delayTimer;
  TLockedToken<CGenDescription> x27c_pickupParticleDesc;

  bool x28c_24_generated : 1 = false;
  bool x28c_25_inTractor : 1 = false;
  bool x28c_26_enableTractorTest : 1 = false;

public:
  CScriptPickup(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                CModelData&& mData, const CActorParameters& aParams, const zeus::CAABox& aabb,
                CPlayerState::EItemType itemType, s32 amount, s32 capacity, CAssetId pickupEffect,
                float possibility, float lifeTime, float fadeInTime, float startDelay, bool active);

  void Accept(IVisitor& visitor) override;
  void Think(float, CStateManager&) override;
  void Touch(CActor&, CStateManager&) override;
  std::optional<zeus::CAABox> GetTouchBounds() const override { return CPhysicsActor::GetBoundingBox(); }
  float GetPossibility() const { return x264_possibility; }
  CPlayerState::EItemType GetItem() const { return x258_itemType; }
  void SetGenerated() { x28c_24_generated = true; }
};
} // namespace urde
