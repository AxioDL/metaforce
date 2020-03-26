#include "Runtime/MP1/World/CElitePirate.hpp"

#include "Runtime/Collision/CCollisionActor.hpp"
#include "Runtime/Collision/CCollisionActorManager.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/ScriptLoader.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {
namespace {
static constexpr std::array<SJointInfo, 3> skJointInfoL{{
    {"L_shoulder", "L_elbow", 1.f, 1.5f},
    {"L_wrist", "L_elbow", 0.9f, 1.3f},
    {"L_knee", "L_ankle", 0.9f, 1.3f},
}};

static constexpr std::array<SJointInfo, 3> skJointInfoR{{
    {"R_shoulder", "R_elbow", 1.f, 1.5f},
    {"R_wrist", "R_elbow", 0.9f, 1.3f},
    {"R_knee", "R_ankle", 0.9f, 1.3f},
}};

static constexpr std::array<SSphereJointInfo, 7> skSphereJointInfo{{
    {"Head_1", 1.2f},
    {"L_Palm_LCTR", 1.5f},
    {"R_Palm_LCTR", 1.5f},
    {"Spine_1", 1.5f},
    {"Collar", 1.2f},
    {"L_Ball", 0.8f},
    {"R_Ball", 0.8f},
}};
} // namespace

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
, x90_launcherParams(ScriptLoader::LoadAnimationParameters(in))
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
, xf6_(CSfxManager::TranslateSFXID(in.readUint32Big()))
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
, x56c_vulnerability(pInfo.GetDamageVulnerability())
, x5d8_data(eliteData)
, x6f8_boneTracking(*GetModelData()->GetAnimationData(), "Head_1", zeus::degToRad(80.f), zeus::degToRad(180.f),
                    EBoneTrackingFlags::None)
, x738_(GetBoundingBox(), GetMaterialList())
, x7d0_pathFindSearch(nullptr, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, x8c0_(5.f)
, x988_24_(false)
, x988_25_(false)
, x988_26_(false)
, x988_27_(false)
, x988_28_alert(false)
, x988_29_(false)
, x988_30_(false)
, x988_31_(false)
, x989_24_(false) {
  if (x5d8_data.GetX20().IsValid()) {
    x760_ = g_SimplePool->GetObj({SBIG('PART'), x5d8_data.GetX20()});
  }

  x460_knockBackController.SetEnableFreeze(false);
  x460_knockBackController.SetAutoResetImpulse(false);
  x460_knockBackController.SetEnableBurn(false);
  x460_knockBackController.SetEnableExplodeDeath(false);
  x460_knockBackController.SetEnableLaggedBurnDeath(false);
  sub_80229248();
}
void CElitePirate::Accept(IVisitor& visitor) { visitor.Visit(this); }
void CElitePirate::Think(float dt, CStateManager& mgr) { CPatterned::Think(dt, mgr); }
void CElitePirate::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  case EScriptObjectMessage::Activate: {
    if (sub_802273a8()) {
      x730_collisionActorMgr2->SetActive(mgr, true);
    }
    if (CEntity* ent = mgr.ObjectById(x772_)) {
      ent->SetActive(true);
    }
    break;
  }
  case EScriptObjectMessage::Deactivate: {
    if (sub_802273a8()) {
      x730_collisionActorMgr2->SetActive(mgr, false);
    }
    x5d4_collisionActorMgr1->SetActive(mgr, false);
    if (CEntity* ent = mgr.ObjectById(x772_)) {
      ent->SetActive(false);
    }
    break;
  }
  case EScriptObjectMessage::Alert:
    x988_28_alert = true;
    break;
  case EScriptObjectMessage::Touched: {
    if (HealthInfo(mgr)->GetHP() <= 0.f)
      break;
    TCastToPtr<CCollisionActor> actor = mgr.ObjectById(uid);
    if (!actor) {
      if (uid == x772_ && x772_ != kInvalidUniqueId) {
        sub_8022759c(true, mgr);
      }
      break;
    }
    const TUniqueId& touchedUid = actor->GetLastTouchedObject();
    if (touchedUid != GetUniqueId()) {
      if (TCastToPtr<CGameProjectile> projectile = mgr.ObjectById(touchedUid)) {
        sub_8022759c(true, mgr);
      }
      break;
    }
    if (!x988_31_) { // TODO is this right?
      if (x420_curDamageRemTime <= 0.f) {
        CDamageInfo info = GetContactDamage();
        info.SetDamage(0.5f * info.GetDamage());
        mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer().GetUniqueId(), GetUniqueId(), info,
                        CMaterialFilter::MakeInclude({EMaterialTypes::Solid}), zeus::skZero3f);
        x420_curDamageRemTime = x424_damageWaitTime;
      }
      break;
    }
    if ((!x988_25_ || !sub_802293f8(uid, x774_collisionRJointIds)) &&
        (!x988_26_ || !sub_802293f8(uid, x788_collisionLJointIds))) {
      break;
    }
    mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer().GetUniqueId(), GetUniqueId(), GetContactDamage(),
                    CMaterialFilter::MakeInclude({EMaterialTypes::Solid}), zeus::skZero3f);
    x420_curDamageRemTime = x424_damageWaitTime;
    x988_24_ = false;
    break;
  }
  case EScriptObjectMessage::Registered:
    x450_bodyController->Activate(mgr);
    SetupCollisionManagers(mgr);
    // TODO
    break;
  default:
    break;
  }

  // TODO

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
const CCollisionPrimitive* CElitePirate::GetCollisionPrimitive() const { return &x738_; }
void CElitePirate::KnockBack(const zeus::CVector3f& pos, CStateManager& mgr, const CDamageInfo& info,
                             EKnockBackType type, bool inDeferred, float magnitude) {
  CPatterned::KnockBack(pos, mgr, info, type, inDeferred, magnitude);
}
void CElitePirate::TakeDamage(const zeus::CVector3f& pos, float) {}
void CElitePirate::Patrol(CStateManager& mgr, EStateMsg msg, float dt) { CPatterned::Patrol(mgr, msg, dt); }
void CElitePirate::PathFind(CStateManager& mgr, EStateMsg msg, float dt) { CPatterned::PathFind(mgr, msg, dt); }
void CElitePirate::TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) { CPatterned::TargetPatrol(mgr, msg, dt); }
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
void CElitePirate::sub_80229114(CStateManager& mgr) {}
void CElitePirate::sub_802289b0(CStateManager& mgr, bool b) {}

void CElitePirate::sub_80229248() {
  float scale = 1.5f * GetModelData()->GetScale().y();
  float fVar1 = sub_802273b0() ? 5.f : 1.f;
  zeus::CAABox box{{-scale, -scale, 0.f}, {scale, scale, fVar1 * scale}};
  SetBoundingBox(box);
  x738_.SetBox(box);
  x7d0_pathFindSearch.SetCharacterRadius(scale);
  x7d0_pathFindSearch.SetCharacterHeight(3.f * scale);
}

void CElitePirate::sub_8022759c(bool param_1, CStateManager& mgr) {
  if (!sub_802273b0() || x7b4_ <= 0.f || !param_1) {
    x988_27_ = param_1;
  } else if (HealthInfo(mgr)->GetHP() / x7b4_ <= x7b0_) {
    x7b0_ -= 0.2f;
    x988_27_ = true;
  }
  if (x988_27_) {
    x7c0_ = mgr.GetActiveRandom()->Float() * x5d8_data.x18_ + x5d8_data.x14_;
  } else {
    x7c0_ = 0.f;
  }
}

bool CElitePirate::sub_802293f8(TUniqueId uid, const rstl::reserved_vector<TUniqueId, 8>& vec) const {
  return std::find(vec.begin(), vec.end(), uid) != vec.end();
}

void CElitePirate::AddCollisionList(const SJointInfo* joints, size_t count,
                                    std::vector<CJointCollisionDescription>& outJoints) const {
  const CAnimData* animData = GetModelData()->GetAnimationData();

  for (size_t i = 0; i < count; ++i) {
    const auto& joint = joints[i];
    const CSegId from = animData->GetLocatorSegId(joint.from);
    const CSegId to = animData->GetLocatorSegId(joint.to);

    if (to.IsInvalid() || from.IsInvalid()) {
      continue;
    }

    outJoints.emplace_back(CJointCollisionDescription::SphereSubdivideCollision(
        to, from, joint.radius, joint.separation, CJointCollisionDescription::EOrientationType::One, joint.from, 10.f));
  }
}

void CElitePirate::AddSphereCollisionList(const SSphereJointInfo* joints, size_t count,
                                          std::vector<CJointCollisionDescription>& outJoints) const {
  const CAnimData* animData = GetModelData()->GetAnimationData();

  for (size_t i = 0; i < count; ++i) {
    const auto& joint = joints[i];
    const CSegId seg = animData->GetLocatorSegId(joint.name);

    if (seg.IsInvalid()) {
      continue;
    }

    outJoints.emplace_back(CJointCollisionDescription::SphereCollision(seg, joint.radius, joint.name, 10.f));
  }
}

void CElitePirate::SetupCollisionManagers(CStateManager& mgr) {
  constexpr size_t jointInfoCount = skJointInfoL.size() + skJointInfoR.size() + skSphereJointInfo.size();
  std::vector<CJointCollisionDescription> joints(jointInfoCount);
  AddCollisionList(skJointInfoL.data(), skJointInfoL.size(), joints);
  AddCollisionList(skJointInfoR.data(), skJointInfoL.size(), joints);
  AddSphereCollisionList(skSphereJointInfo.data(), skSphereJointInfo.size(), joints);
  if (sub_802273a8()) {
    x730_collisionActorMgr2 =
        std::make_unique<CCollisionActorManager>(mgr, GetUniqueId(), GetAreaIdAlways(), joints, true);
    x730_collisionActorMgr2->SetActive(mgr, GetActive());
  }
  x774_collisionRJointIds.clear();
  x788_collisionLJointIds.clear();

  const CAnimData* animData = GetModelData()->GetAnimationData();
  constexpr zeus::CVector3f bounds{4.f, 4.f, 2.f};
  joints.emplace_back(CJointCollisionDescription::OBBCollision(animData->GetLocatorSegId("L_Palm_LCTR"sv), bounds,
                                                               zeus::skZero3f, "Shield"sv, 10.f));
  x5d4_collisionActorMgr1 =
      std::make_unique<CCollisionActorManager>(mgr, GetUniqueId(), GetAreaIdAlways(), joints, false);

  sub_80229818(mgr);
  sub_80229114(mgr);

  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      {EMaterialTypes::Solid},
      {EMaterialTypes::CollisionActor, EMaterialTypes::AIPassthrough, EMaterialTypes::Player}));
  AddMaterial(EMaterialTypes::ProjectilePassthrough, mgr);
}

void CElitePirate::sub_80229818(CStateManager& mgr) {
  if (sub_802273a8()) {
    for (size_t i = 0; i < x730_collisionActorMgr2->GetNumCollisionActors(); ++i) {
      const auto& colDesc = x730_collisionActorMgr2->GetCollisionDescFromIndex(i);
      const TUniqueId& uid = colDesc.GetCollisionActorId();
      if (TCastToPtr<CCollisionActor> act = mgr.ObjectById(uid)) {
        if (colDesc.GetName() == "Head_1"sv) {
          x770_collisionHeadId = uid;
        } else if (sub_8022943c(colDesc.GetName(), "R_Palm_LCTR"sv, skJointInfoR.data(), skJointInfoR.size())) {
          x774_collisionRJointIds.push_back(uid);
        } else if (sub_8022943c(colDesc.GetName(), "L_Palm_LCTR"sv, skJointInfoL.data(), skJointInfoL.size())) {
          x788_collisionLJointIds.push_back(uid);
        }
        if (uid != x770_collisionHeadId) {
          act->SetDamageVulnerability(CDamageVulnerability::ReflectVulnerabilty());
        }
      }
    }
    x730_collisionActorMgr2->AddMaterial(
        mgr, {EMaterialTypes::AIJoint, EMaterialTypes::CameraPassthrough, EMaterialTypes::Immovable});
  }

  const CJointCollisionDescription& description = x5d4_collisionActorMgr1->GetCollisionDescFromIndex(0);
  x79c_ = description.GetCollisionActorId();
  if (TCastToPtr<CCollisionActor> act = mgr.ObjectById(x79c_)) {
    act->SetWeaponCollisionResponseType(EWeaponCollisionResponseTypes::None);
  }
  x5d4_collisionActorMgr1->AddMaterial(mgr, {EMaterialTypes::AIJoint, EMaterialTypes::CameraPassthrough});
}

bool CElitePirate::sub_8022943c(std::string_view name, std::string_view locator, const SJointInfo* info,
                                size_t infoCount) {
  if (name == locator) {
    return true;
  }
  for (size_t i = 0; i < infoCount; ++i) {
    if (name == info[i].from) {
      return true;
    }
  }
  return false;
}

void CElitePirate::CreateGrenadeLauncher(CStateManager& mgr, TUniqueId uid) {
  CAnimationParameters& params = x5d8_data.x90_launcherParams;
  if (!params.GetACSFile().IsValid()) {
    return;
  }
  CModelData data(CAnimRes(params.GetACSFile(), params.GetCharacter(), GetModelData()->GetScale(),
                           params.GetInitialAnimation(), true));
  // TODO
}
} // namespace urde::MP1
