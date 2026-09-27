#include "MetroidPrime/Enemies/CBurrower.hpp"

#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "Kyoto/Animation/CVertexMorphEffect.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "Kyoto/Particles/CElementGen.hpp"

const CDamageVulnerability CBurrower::skBombVulnerability =
    CDamageVulnerability(kVN_Deflect, kVN_Deflect, kVN_Deflect, kVN_Deflect, kVN_Normal, kVN_Normal,
                        kVN_Deflect, kVN_Deflect, kVN_Deflect, kVN_Deflect, kVN_Deflect,
                        kVN_Deflect, kVN_Deflect, kVN_Deflect, kVN_Deflect, kDT_None);

CBurrower::CBurrower(const TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                    const CTransform4f& xf, const CModelData& mData, const CPatternedInfo& pInfo,
                    const CActorParameters& actParms, const CAssetId& jumpParticle,
                    const CAssetId& trailParticle, const CAssetId& projectile,
                    const CDamageInfo& projectileDamage, const CAssetId& visorParticle,
                    const uint visorSfx, const CAssetId deathExplosionParticle)
: CPatterned(kC_Burrower, uid, name, kFT_Zero, info, xf, mData, pInfo, kMT_Ground, kCT_One,
             kBT_BiPedal, actParms, kCS_Small)
, mPathFindSearch(nullptr, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, mProjectileInfo(projectile, projectileDamage)
, mAttackTime(0.f)
, mLurkTimer(0.f)
, mInvulnDamageTime(0.f)
, mLastDestObj(kInvalidUniqueId)
, mVisorSfx(CSfxManager::TranslateSFXID(visorSfx))
, mDoFacePlayer(false)
, mInAir(false) {
  SetDrawShadow(false);
  MakeThermalColdAndHot();
  mProjectileInfo.Token().Lock();

  if (jumpParticle != kInvalidAssetId) {
    TLockedToken< CGenDescription > desc =
        TLockedToken< CGenDescription >(gpSimplePool->GetObj(SObjectTag('PART', jumpParticle)));
    mJumpParticle = rs_new CElementGen(desc, CElementGen::kMOT_One);
    if (mJumpParticle.get()) {
      mJumpParticle->SetLeaveLightsEnabledForModelRender(true);
      mJumpParticle->SetGlobalScale(GetModelScale());
      mJumpParticle->SetParticleEmission(false);
    }
  }

  if (trailParticle != kInvalidAssetId) {
    TLockedToken< CGenDescription > desc =
        TLockedToken< CGenDescription >(gpSimplePool->GetObj(SObjectTag('PART', trailParticle)));
    mTrailParticle = rs_new CElementGen(desc);
    if (mTrailParticle.get()) {
      mTrailParticle->SetGlobalScale(GetModelScale());
      mTrailParticle->SetParticleEmission(false);
    }
  }

  if (visorParticle != kInvalidAssetId) {
    mVisorParticle = rstl::optional_object< TLockedToken< CGenDescription > >(
        gpSimplePool->GetObj(SObjectTag('PART', visorParticle)));
  }

  if (deathExplosionParticle != kInvalidAssetId) {
    mDeathExplosionParticle = gpSimplePool->GetObj(SObjectTag('PART', deathExplosionParticle));
    mDeathExplosionParticle->Lock();
  }
}

CBurrower::~CBurrower() {}

void CBurrower::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
  switch (msg) {
  case kSM_Registered:
    BodyCtrl()->Activate(mgr);
    break;
  case kSM_InitializedInArea: {
    const TAreaId areaId = GetCurrentAreaId();
    const CGameArea::CPostConstructed* constructed =
        mgr.GetWorld()->GetAreaAlways(areaId).GetPostConstructed();
    mPathFindSearch.SetArea(constructed->mPathArea);
    if (!CPatterned::HasPatrolPath(mgr, 0.f)) {
      mTrailParticle = nullptr;
    }
    break;
  }
  case kSM_InvulnDamage:
    mInvulnDamageTime = 1.f;
    break;
  }
}

void CBurrower::Render(const CStateManager& mgr) const {
  if (GetActorLights() && mJumpParticle.get()) {
    GetActorLights()->ActivateLights();
    mJumpParticle->Render();
  }
  CPatterned::Render(mgr);
}

void CBurrower::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  CPatterned::Think(dt, mgr);
  if (mInvulnDamageTime > 0.f) {
    mInvulnDamageTime -= dt;
  }
  if (mDoFacePlayer) {
    CVector3f forward = GetTransform().GetForward();
    CVector3f toPlayer = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    forward.SetZ(0.f);
    forward.Normalize();
    toPlayer.SetZ(0.f);
    toPlayer.Normalize();
    if (CVector3f::Dot(forward, toPlayer) < 0.9993f) {
      const CQuaternion rotation = CQuaternion::LookAt(
          CUnitVector3f(forward, CUnitVector3f::kN_No),
          CUnitVector3f(toPlayer[kDX], toPlayer[kDY], toPlayer[kDZ]), CRelAngle::FromDegrees(360.f * dt));
      RotateToOR(CQuaternion(rotation.GetScalar(),
                             GetTransform().TransposeRotate(rotation.GetVector())), dt);
    }
  } else if (mAttackTime > 0.f) {
    mAttackTime -= dt;
  }
  if (mJumpParticle.get()) {
    mLurkTimer -= dt;
    if (!mInAir && mLurkTimer <= 0.f) {
      if (IsAlive()) {
        mJumpParticle->SetParticleEmission(true);
        mJumpParticle->SetOrientation(GetTransform().GetRotation());
        mJumpParticle->SetTranslation(GetTranslation());
        mJumpParticle->ForceParticleCreation(1);
        mJumpParticle->SetOrientation(CTransform4f::Identity());
        mJumpParticle->SetParticleEmission(false);
      }
      mLurkTimer = 0.1875f;
    }
    mJumpParticle->Update(dt);
  }
  if (mTrailParticle.get()) {
    if (IsAlive() && !mInAir) {
      mTrailParticle->SetTranslation(GetTranslation());
    }
    mTrailParticle->Update(dt);
  }
}

void CBurrower::Patrol(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mInAir = false;
    if (mTrailParticle.get()) {
      mTrailParticle->SetParticleEmission(true);
    }
    break;
  case kStateMsg_Update:
    break;
  case kStateMsg_Deactivate:
    mLastDestObj = mDestObj;
    break;
  }
  CPatterned::Patrol(mgr, msg, arg);
}

void CBurrower::Active(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mDoFacePlayer = true;
    mAnimState = kAS_Ready;
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Generate, &CPatterned::TryGenerateDeactivate, 0);
    break;
  case kStateMsg_Deactivate:
    mDoFacePlayer = false;
    mInAir = true;
    mAnimState = kAS_NotReady;
    break;
  }
}

void CBurrower::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mDoFacePlayer = true;
    mInAir = true;
    if (mTrailParticle.get()) {
      mTrailParticle->SetParticleEmission(false);
    }
    mAnimState = kAS_Ready;
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_ProjectileAttack, &CPatterned::TryProjectileAttack, 0);
    break;
  case kStateMsg_Deactivate:
    mAnimState = kAS_NotReady;
    mAttackTime = mgr.Random()->Float() * mAttackTimeVariation + mAverageAttackTime;
    mDoFacePlayer = false;
    mVerticalMovement = false;
    AddMaterial(kMT_GroundCollider, mgr);
    mMoveScale = CVector3f(1.f, 1.f, 1.f);
    break;
  }
}

void CBurrower::Retreat(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimState = kAS_Ready;
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Generate, &CPatterned::TryGenerateDeactivate, 1);
    break;
  case kStateMsg_Deactivate:
    mAnimState = kAS_NotReady;
    if (mTrailParticle.get()) {
      mInAir = false;
      mTrailParticle->SetParticleEmission(true);
    }
    break;
  }
}

void CBurrower::TargetPatrol(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    const TUniqueId dest = mLastDestObj != kInvalidUniqueId
                               ? mLastDestObj
                               : GetConnectedObject(mgr, kSS_Patrol, kSM_Follow);
    mDestObj = dest;
    if (const CActor* actor = TCastToConstPtr< CActor >(mgr.GetObjectById(dest))) {
      mDestPos = actor->GetTranslation();
      mInPosition = false;
      mReflectedDestPos = GetTranslation();
    }
    break;
  }
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
}

void CBurrower::Lurk(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mInAir = false;
    mLurkTimer = 0.1875f;
    break;
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
}

CProjectileInfo* CBurrower::ProjectileInfo() { return &mProjectileInfo; }

void CBurrower::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                               EUserEventType type, float dt) {
  bool handled = false;
  switch (type) {
  case kUE_Projectile: {
    const CPlayer& player = *mgr.GetPlayer();
    const CVector3f aimPos = player.GetAimPosition(mgr, 0.f);
    const CVector3f gunPos = GetLctrTransform(node.GetLocatorName()).GetTranslation();
    const CVector3f target = ProjectileInfo()->PredictInterceptPos(gunPos, aimPos, player, true, dt);
    LaunchProjectile(CTransform4f::LookAt(gunPos, target), mgr, 1, CWeapon::kPA_None, false,
                     mVisorParticle, mVisorSfx, false, GetModelScale());
    handled = true;
    break;
  }
  case kUE_TakeOff:
    RemoveMaterial(kMT_GroundCollider, mgr);
    mVerticalMovement = true;
    mMoveScale = GetModelScale();
    handled = true;
    break;
  case kUE_Landing:
    mVerticalMovement = false;
    AddMaterial(kMT_GroundCollider, mgr);
    mMoveScale = CVector3f(1.f, 1.f, 1.f);
    handled = true;
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

bool CBurrower::PathShagged(CStateManager& mgr, float arg) {
  switch (mPathFindSearch.OnPath(GetTranslation())) {
  case CPathFindSearch::kR_InvalidArea:
    return true;
  default:
    return false;
  }
}

bool CBurrower::ShouldAttack(CStateManager& mgr, float arg) {
  if (mInvulnDamageTime <= 0.f) {
    return mAttackTime <= 0.f && mgr.CanCreateProjectile(GetUniqueId(), kWT_AI, 1);
  }
  return false;
}

const CDamageVulnerability* CBurrower::GetDamageVulnerability() const {
  if (mInAir != true) {
    return &skBombVulnerability;
  }
  return CAi::GetDamageVulnerability();
}

const CDamageVulnerability* CBurrower::GetDamageVulnerability(
    const CVector3f&, const CVector3f&, const CDamageInfo&) const {
  if (mInAir != true) {
    return &skBombVulnerability;
  }
  return CAi::GetDamageVulnerability();
}

void CBurrower::AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const {
  if (GetActive() && mTrailParticle.get()) {
    gpRender->AddParticleGen(*mTrailParticle);
  }
  CPatterned::AddToRenderer(frustum, mgr);
}

void CBurrower::Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) {
  if (IsAlive()) {
    CPatterned::Death(mgr, direction, state);
    if (mTrailParticle.get()) {
      mTrailParticle->SetParticleEmission(false);
    }
  }
}

void CBurrower::TurnAround(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    const CVector3f forward = GetTransform().GetForward();
    BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(-forward, forward, 1.f));
    break;
  }
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
}

const rstl::optional_object< TCachedToken< CGenDescription > >&
CBurrower::GetDeathExplosionParticle() const {
  return mInAir ? mDeathExplosionParticle : CPatterned::mDeathExplosionParticle;
}
