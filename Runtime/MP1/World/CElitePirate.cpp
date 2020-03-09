#include "Runtime/MP1/World/CElitePirate.hpp"

#include "Runtime/Collision/CCollisionActorManager.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/World/ScriptLoader.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {
CElitePirateData::CElitePirateData(CInputStream& in, u32 propCount)
: x0_(in.readFloatBig())
, x4_(in.readFloatBig())
, x8_(in.readFloatBig())
, xc_(in.readFloatBig())
, x10_(in.readFloatBig())
, x14_(in.readFloatBig())
, x18_(in.readFloatBig())
, x1c_(in.readFloatBig())
, x20_(in)
, x24_(CSfxManager::TranslateSFXID(in.readUint32Big()))
, x28_(ScriptLoader::LoadActorParameters(in))
, x90_(ScriptLoader::LoadAnimationParameters(in))
, x9c_(in)
, xa0_(CSfxManager::TranslateSFXID(in.readUint32Big()))
, xa4_(in)
, xa8_(in)
, xc4_(in.readFloatBig())
, xc8_(in)
, xcc_(in)
, xd0_(in)
, xd4_(in)
, xd8_(in.readFloatBig())
, xdc_(in.readFloatBig())
, xe0_(in.readFloatBig())
, xe4_(in.readFloatBig())
, xe8_(zeus::degToRad(in.readFloatBig()))
, xec_(zeus::degToRad(in.readFloatBig()))
, xf0_(in.readUint32Big())
, xf4_(CSfxManager::TranslateSFXID(in.readUint32Big()))
, xf8_(in)
, xfc_(in)
, x118_(in)
, x11c_(CSfxManager::TranslateSFXID(in.readUint32Big()))
, x11e_(in.readBool())
, x11f_(propCount < 24 ? true : in.readBool()) {}

CElitePirate::CElitePirate(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                           CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms,
                           const CElitePirateData& eliteData)
: CPatterned(ECharacter::ElitePirate, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Ground, EColliderType::One, EBodyType::BiPedal, actParms, EKnockBackVariant::Large)
, x56c_(pInfo.GetDamageVulnerability())
, x5d8_(eliteData)
, x6f8_(*GetModelData()->GetAnimationData(), "Head_1", zeus::degToRad(80.f), zeus::degToRad(180.f),
        EBoneTrackingFlags::None)
, x738_(GetBoundingBox(), GetMaterialList())
, x7d0_(nullptr, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f) {
  if (x5d8_.GetX20().IsValid()) {
    x760_ = g_SimplePool->GetObj({SBIG('PART'), x5d8_.GetX20()});
  }

  x460_knockBackController.SetEnableFreeze(false);
  x460_knockBackController.SetAutoResetImpulse(false);
  x460_knockBackController.SetEnableBurn(false);
  x460_knockBackController.SetEnableExplodeDeath(false);
  x460_knockBackController.SetEnableLaggedBurnDeath(false);
  sub80229248();
}
void CElitePirate::Accept(IVisitor& visitor) { visitor.Visit(this); }
void CElitePirate::Think(float dt, CStateManager& mgr) { CPatterned::Think(dt, mgr); }
void CElitePirate::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
}
void CElitePirate::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) { CPatterned::PreRender(mgr, frustum); }
const CDamageVulnerability* CElitePirate::GetDamageVulnerability() const { return CAi::GetDamageVulnerability(); }
const CDamageVulnerability* CElitePirate::GetDamageVulnerability(const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                                                                 const CDamageInfo& dInfo) const {
  return CActor::GetDamageVulnerability(pos, dir, dInfo);
}
zeus::CVector3f CElitePirate::GetOrbitPosition(const CStateManager& mgr) const {
  return CPatterned::GetOrbitPosition(mgr);
}
zeus::CVector3f CElitePirate::GetAimPosition(const CStateManager& mgr, float dt) const {
  return CPatterned::GetAimPosition(mgr, dt);
}
void CElitePirate::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}
const CCollisionPrimitive* CElitePirate::GetCollisionPrimitive() const {
  return CPhysicsActor::GetCollisionPrimitive();
}
void CElitePirate::KnockBack(const zeus::CVector3f& pos, CStateManager& mgr, const CDamageInfo& info, EKnockBackType type,
                             bool inDeferred, float magnitude) {
  CPatterned::KnockBack(pos, mgr, info, type, inDeferred, magnitude);
}
void CElitePirate::TakeDamage(const zeus::CVector3f& pos, float arg) { CPatterned::TakeDamage(pos, arg); }
void CElitePirate::Patrol(CStateManager& mgr, EStateMsg msg, float dt) { CPatterned::Patrol(mgr, msg, dt); }
void CElitePirate::PathFind(CStateManager& mgr, EStateMsg msg, float dt) { CPatterned::PathFind(mgr, msg, dt); }
void CElitePirate::TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::TargetPatrol(mgr, msg, dt);
}
void CElitePirate::Halt(CStateManager& mgr, EStateMsg msg, float dt) { CAi::Halt(mgr, msg, dt); }
void CElitePirate::Run(CStateManager& mgr, EStateMsg msg, float dt) { CAi::Run(mgr, msg, dt); }
void CElitePirate::Generate(CStateManager& mgr, EStateMsg msg, float dt) { CAi::Generate(mgr, msg, dt); }
void CElitePirate::Attack(CStateManager& mgr, EStateMsg msg, float dt) { CAi::Attack(mgr, msg, dt); }
void CElitePirate::Taunt(CStateManager& mgr, EStateMsg msg, float dt) { CAi::Taunt(mgr, msg, dt); }
void CElitePirate::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  CAi::ProjectileAttack(mgr, msg, dt);
}
void CElitePirate::SpecialAttack(CStateManager& mgr, EStateMsg msg, float dt) { CAi::SpecialAttack(mgr, msg, dt); }
void CElitePirate::CallForBackup(CStateManager& mgr, EStateMsg msg, float dt) { CAi::CallForBackup(mgr, msg, dt); }
bool CElitePirate::TooClose(CStateManager& mgr, float arg) { return CPatterned::TooClose(mgr, arg); }
bool CElitePirate::InDetectionRange(CStateManager& mgr, float arg) { return CPatterned::InDetectionRange(mgr, arg); }
bool CElitePirate::SpotPlayer(CStateManager& mgr, float arg) { return CPatterned::SpotPlayer(mgr, arg); }
bool CElitePirate::AnimOver(CStateManager& mgr, float arg) { return CPatterned::AnimOver(mgr, arg); }
bool CElitePirate::ShouldAttack(CStateManager& mgr, float arg) { return CAi::ShouldAttack(mgr, arg); }
bool CElitePirate::InPosition(CStateManager& mgr, float arg) { return CPatterned::InPosition(mgr, arg); }
bool CElitePirate::ShouldTurn(CStateManager& mgr, float arg) { return CAi::ShouldTurn(mgr, arg); }
bool CElitePirate::AggressionCheck(CStateManager& mgr, float arg) { return CAi::AggressionCheck(mgr, arg); }
bool CElitePirate::ShouldTaunt(CStateManager& mgr, float arg) { return CAi::ShouldTaunt(mgr, arg); }
bool CElitePirate::ShouldFire(CStateManager& mgr, float arg) { return CAi::ShouldFire(mgr, arg); }
bool CElitePirate::ShotAt(CStateManager& mgr, float arg) { return CAi::ShotAt(mgr, arg); }
bool CElitePirate::ShouldSpecialAttack(CStateManager& mgr, float arg) { return CAi::ShouldSpecialAttack(mgr, arg); }
bool CElitePirate::ShouldCallForBackup(CStateManager& mgr, float arg) { return CAi::ShouldCallForBackup(mgr, arg); }
CPathFindSearch* CElitePirate::GetSearchPath() { return CPatterned::GetSearchPath(); }
void CElitePirate::V181(CStateManager& mgr) {}
void CElitePirate::v182(CStateManager& mgr, bool b) {}
} // namespace urde::MP1
