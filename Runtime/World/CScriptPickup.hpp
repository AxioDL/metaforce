#pragma once

#include "CPhysicsActor.hpp"
#include "CPlayerState.hpp"

namespace urde
{
class CScriptPickup : public CPhysicsActor
{
    CPlayerState::EItemType x258_itemType;
    u32 x25c_amount;
    u32 x260_capacity;
    float x264_possibility;
    float x268_;
    float x26c_;
    float x270_ = 0.f;
    float x274_;
    float x278_;
    TLockedToken<CGenDescription> x27c_explosionDesc;

    u8 x28c_24_ : 1;
    u8 x28c_25_ : 1;
    u8 x28c_26_ : 1;

public:
    CScriptPickup(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
                  const CActorParameters&, const zeus::CAABox&, CPlayerState::EItemType, s32, s32, CAssetId, float,
                  float, float, float, bool);

    void Accept(IVisitor& visitor);
    void Think(float, CStateManager&);
    void Touch(CActor &, CStateManager &);
    std::experimental::optional<zeus::CAABox> GetTouchBounds() const
    {
        return CPhysicsActor::GetBoundingBox();
    }
    float GetPossibility() const { return x264_possibility; }
    CPlayerState::EItemType GetItem() { return x258_itemType; }
};
}

