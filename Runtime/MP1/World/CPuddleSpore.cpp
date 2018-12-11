#include "CPuddleSpore.hpp"
#include "World/CPatternedInfo.hpp"
#include "CStateManager.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "TCastTo.hpp"

namespace urde::MP1 {
const std::string_view CPuddleSpore::kEyeLocators[16] = {
  "Glow_1_LCTR"sv,
  "Glow_2_LCTR"sv,
  "Glow_3_LCTR"sv,
  "Glow_4_LCTR"sv,
  "Glow_5_LCTR"sv,
  "Glow_6_LCTR"sv,
  "Glow_7_LCTR"sv,
  "Glow_8_LCTR"sv,
  "Glow_9_LCTR"sv,
  "Glow_10_LCTR"sv,
  "Glow_11_LCTR"sv,
  "Glow_12_LCTR"sv,
  "Glow_13_LCTR"sv,
  "Glow_14_LCTR"sv,
  "Glow_15_LCTR"sv,
  "Glow_16_LCTR"sv,
};
CPuddleSpore::CPuddleSpore(TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo& info,
                           const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo,
                           EColliderType colType, CAssetId glowFx, float f1, float f2, float f3, float f4, float f5,
                           const CActorParameters& actParms, CAssetId weapon, const CDamageInfo& dInfo)
: CPatterned(ECharacter::PuddleSpore, uid, name, flavor, info, xf, std::move(mData), pInfo, EMovementType::Flyer,
             colType, EBodyType::Restricted, actParms, EKnockBackVariant::Medium)
, x570_(f1)
, x574_(f2)
, x578_(f3)
, x57c_(f4)
, x580_(f5)
, x584_bodyOrigin(pInfo.GetBodyOrigin())
, x590_halfExtent(pInfo.GetHalfExtent())
, x594_height(pInfo.GetHeight())
, x5a0_(CalculateBoundingBox(), GetMaterialList())
, x5ec_projectileInfo(weapon, dInfo)
, x5d0_(g_SimplePool->GetObj({SBIG('PART'), glowFx}))
, x614_24(false)
, x614_25(false) {
  x5dc_.reserve(kEyeCount);
  for (u32 i = 0; i < kEyeCount; ++i)
    x5dc_.emplace_back(new CElementGen(x5d0_));
  const_cast<TToken<CWeaponDescription>*>(&x5ec_projectileInfo.Token())->Lock();
  x460_knockBackController.SetAutoResetImpulse(false);
}

zeus::CAABox CPuddleSpore::CalculateBoundingBox() const {
  return {((zeus::CVector3f(-x590_halfExtent, -x590_halfExtent, x598_) + x584_bodyOrigin) * 0.5f) +
          (GetBaseBoundingBox().min * 0.95f),
          ((zeus::CVector3f(x590_halfExtent, x590_halfExtent, (x594_height * x59c_) + x598_) + x584_bodyOrigin) * 0.5f) +
          (GetBaseBoundingBox().max * 0.95f)};
}

void CPuddleSpore::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  if (msg == EScriptObjectMessage::InvulnDamage)
    return;
  if (msg == EScriptObjectMessage::Registered)
    x450_bodyController->Activate(mgr);

  CPatterned::AcceptScriptMsg(msg, uid, mgr);
}
}