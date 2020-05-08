#include "Runtime/MP1/World/CMetroid.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/Character/CPASAnimParmData.hpp"
#include "Runtime/Weapon/CGameProjectile.hpp"
#include "Runtime/World/CTeamAiMgr.hpp"
#include "Runtime/World/CWorld.hpp"
#include "Runtime/World/ScriptLoader.hpp"

namespace urde::MP1 {

CMetroidData::CMetroidData(CInputStream& in)
: x0_dVuln1(in)
, x68_dVuln2(in)
, xd0_(in.readFloatBig())
, xd4_(in.readFloatBig())
, xd8_(in.readFloatBig())
, xdc_(in.readFloatBig())
, xe0_(in.readFloatBig())
, xe4_(in.readFloatBig()) {
  xe8_animParms1 = ScriptLoader::LoadAnimationParameters(in);
  xf8_animParms2 = ScriptLoader::LoadAnimationParameters(in);
  x108_animParms3 = ScriptLoader::LoadAnimationParameters(in);
  x118_animParms4 = ScriptLoader::LoadAnimationParameters(in);
  x128_24_startsInWall = in.readBool();
}

CMetroid::CMetroid(TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo& info,
                   const zeus::CTransform& xf, CModelData&& mData, const CPatternedInfo& pInfo,
                   const CActorParameters& aParms, const CMetroidData& metroidData, TUniqueId other)
: CPatterned(ECharacter::Metroid, uid, name, flavor, info, xf, std::move(mData), pInfo, EMovementType::Flyer,
             EColliderType::One, EBodyType::Flyer, aParms, EKnockBackVariant::Medium)
, x56c_data(metroidData)
, x6a0_collisionPrimitive(zeus::CSphere{zeus::skZero3f, 0.9f * GetModelData()->GetScale().y()}, GetMaterialList())
, x6c0_pathFindSearch(nullptr, 3, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, x7cc_animParmsidx(flavor == EFlavorType::Two ? 0 : 1)
, x7d0_scale1(GetModelData()->GetScale())
, x7dc_scale2(GetModelData()->GetScale())
, x7e8_scale3(GetModelData()->GetScale())
, x81c_patternedInfo(pInfo)
, x954_actParams(aParms)
, x9bc_(other) {
  x808_loopAttackDistance =
      GetAnimationDistance(CPASAnimParmData{9, CPASAnimParm::FromEnum(2), CPASAnimParm::FromEnum(3)});
  UpdateTouchBounds();
  SetCoefficientOfRestitutionModifier(0.9f);
  x460_knockBackController.SetX82_24(false);
  x460_knockBackController.SetEnableBurn(false);
  x460_knockBackController.SetEnableBurnDeath(false);
  x460_knockBackController.SetEnableShock(false);
  if (flavor == CPatterned::EFlavorType::Two) {
    x460_knockBackController.SetEnableFreeze(false);
  }
  x81c_patternedInfo.SetActive(true);
}

void CMetroid::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
  switch (msg) {
  case EScriptObjectMessage::Registered:
    x450_bodyController->Activate(mgr);
    UpdateVolume();
    break;
  case EScriptObjectMessage::Alert:
    x9bf_24_alert = true;
    break;
  case EScriptObjectMessage::Deactivate:
    SwarmRemove(mgr);
    break;
  case EScriptObjectMessage::Damage:
    if (TCastToConstPtr<CGameProjectile> projectile = mgr.GetObjectById(uid)) {
      const CDamageInfo& damageInfo = projectile->GetDamageInfo();
      if (GetDamageVulnerability()->WeaponHits(damageInfo.GetWeaponMode(), false)) {
        ApplyGrowth(damageInfo.GetDamage());
      }
    }
    x9bf_24_alert = true;
    break;
  case EScriptObjectMessage::InitializedInArea:
    if (x698_teamAiMgrId == kInvalidUniqueId) {
      x698_teamAiMgrId = CTeamAiMgr::GetTeamAiMgr(*this, mgr);
    }
    x6c0_pathFindSearch.SetArea(mgr.GetWorld()->GetAreaAlways(GetAreaIdAlways())->GetPostConstructed()->x10bc_pathArea);
    break;
  default:
    break;
  }
}

void CMetroid::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  if (CTeamAiMgr::GetTeamAiRole(mgr, x698_teamAiMgrId, GetUniqueId()) == nullptr) {
    SwarmAdd(mgr);
  }
  UpdateAttackChance(mgr, dt);
  SuckEnergyFromTarget(mgr, dt);
  PreventWorldCollisions(mgr, dt);
  UpdateTouchBounds();
  RestoreSolidCollision(mgr);
  CPatterned::Think(dt, mgr);
}

void CMetroid::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType eType, float dt) {
  if (eType == EUserEventType::GenerateEnd) {
    AddMaterial(EMaterialTypes::Solid, mgr);
  } else {
    CPatterned::DoUserAnimEvent(mgr, node, eType, dt);
  }
}

EWeaponCollisionResponseTypes CMetroid::GetCollisionResponseType(const zeus::CVector3f& vec1,
                                                                 const zeus::CVector3f& vec2, const CWeaponMode& mode,
                                                                 EProjectileAttrib attribute) const {
  EWeaponCollisionResponseTypes types = EWeaponCollisionResponseTypes::Unknown33;
  if (!GetDamageVulnerability()->WeaponHurts(mode, false) && x450_bodyController->GetPercentageFrozen() <= 0.f) {
    types = EWeaponCollisionResponseTypes::Unknown58;
  }
  return types;
}

} // namespace urde::MP1
