#include "MP1/World/CEnergyBall.hpp"

namespace urde::MP1 {
CEnergyBall::CEnergyBall(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                         CModelData&& mData, const CActorParameters& actParms, const CPatternedInfo& pInfo, s32 w1,
                         float f1, const CDamageInfo& dInfo1, float f2, const CAssetId& a1, s16 sfxId1,
                         const CAssetId& a2, const CAssetId& a3, s16 sfxId2, float f3, float f4, const CAssetId& a4,
                         const CDamageInfo& dInfo2, float f5)
: CPatterned(ECharacter::EnergyBall, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Flyer, EColliderType::One, EBodyType::NewFlyer, actParms, EKnockBackVariant::Medium) {}
}