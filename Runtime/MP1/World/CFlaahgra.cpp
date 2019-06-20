#include "CFlaahgra.hpp"
#include "CDependencyGroup.hpp"
#include "Character/CBoneTracking.hpp"
#include "Collision/CCollisionActor.hpp"
#include "Collision/CCollisionActorManager.hpp"
#include "MP1/World/CFlaahgraProjectile.hpp"
#include "Particle/CElementGen.hpp"
#include "World/CActorParameters.hpp"
#include "World/CGameArea.hpp"
#include "World/CPlayer.hpp"
#include "World/CScriptWaypoint.hpp"
#include "World/CWorld.hpp"
#include "World/ScriptLoader.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"

namespace urde::MP1 {

const SJointInfo CFlaahgra::skLeftArmJointList[3]{
    {"L_elbow", "L_blade", 0.6f, 1.f}, {"L_blade", "L_claw", 0.6f, 1.f}, {"L_CLAW_LCTR", "L_CLAW_END_LCTR", 0.6f, 1.f}};

const SJointInfo CFlaahgra::skRightArmJointList[3]{
    {"R_elbow", "R_blade", 0.6f, 1.f}, {"R_blade", "R_claw", 0.6f, 1.f}, {"R_CLAW_LCTR", "R_CLAW_END_LCTR", 0.6f, 1.f}};
const SSphereJointInfo CFlaahgra::skSphereJointList[5]{
    {"Head_1", 1.5f}, {"Spine_2", 1.5f}, {"Spine_4", 1.5f}, {"Spine_6", 1.5f}, {"Collar", 1.5f}};

CFlaahgraData::CFlaahgraData(CInputStream& in)
: x0_(in.readFloatBig())
, x4_(in.readFloatBig())
, x8_(in.readFloatBig())
, xc_(in.readFloatBig())
, x10_(in)
, x78_(in)
, x7c_(in)
, x98_(in)
, x9c_(in)
, xb8_(in)
, xbc_(in)
, xd8_(ScriptLoader::LoadActorParameters(in))
, x140_(in.readFloatBig())
, x144_(in.readFloatBig())
, x148_(in.readFloatBig())
, x14c_animationParameters(ScriptLoader::LoadAnimationParameters(in))
, x158_(in) {}

CFlaahgraRenderer::CFlaahgraRenderer(TUniqueId uid, TUniqueId owner, std::string_view name, const CEntityInfo& info,
                                     const zeus::CTransform& xf)
: CActor(uid, true, name, info, xf, CModelData::CModelDataNull(), CMaterialList(EMaterialTypes::Character),
         CActorParameters::None(), kInvalidUniqueId)
, xe8_owner(owner) {}

void CFlaahgraRenderer::AddToRenderer(const zeus::CFrustum& frustum, const CStateManager& mgr) const {

  if (const CActor* act = static_cast<const CActor*>(mgr.GetObjectById(xe8_owner))) {
    if (act->HasModelData() && (act->GetModelData()->HasAnimData() || act->GetModelData()->HasNormalModel()))
      act->GetModelData()->RenderParticles(frustum);
  }
}
void CFlaahgraRenderer::Accept(IVisitor& visitor) { visitor.Visit(this); }

CFlaahgra::CFlaahgra(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                     const CAnimRes& animRes, const CPatternedInfo& pInfo, const CActorParameters& actParms,
                     const CFlaahgraData& flaahgraData)
: CPatterned(ECharacter::Flaahgra, uid, name, EFlavorType::Zero, info, xf, CModelData::CModelDataNull(), pInfo,
             EMovementType::Flyer, EColliderType::One, EBodyType::Restricted, actParms, EKnockBackVariant::Large)
, x56c_(flaahgraData)
, x6d4_(g_SimplePool->GetObj({SBIG('PART'), x56c_.xb8_}))
, x6dc_(x56c_.x78_, x56c_.x7c_)
, x704_(x56c_.x98_, x56c_.x9c_)
, x7dc_(GetContactDamage())
, x820_(xf.origin)
, x8a0_(xf.frontVector())
, x8ac_(animRes) {
  x8e5_29_ = true;
  x6dc_.Token().Lock();
  x704_.Token().Lock();
  x7dc_.SetDamage(0.5f * x7dc_.GetDamage());
  SetActorLights(actParms.GetLightParameters().MakeActorLights());
  x90_actorLights->SetCastShadows(false);
  x90_actorLights->SetMaxAreaLights(2);
  x90_actorLights->SetHasAreaLights(x90_actorLights->GetMaxAreaLights() > 0);
  x90_actorLights->SetMaxDynamicLights(1);
  x460_knockBackController.SetAutoResetImpulse(false);
  x460_knockBackController.SetEnableLaggedBurnDeath(false);
  x430_damageColor = skDamageColor;
  LoadDependencies(x56c_.x158_);
  /* TODO: Math that makes me go cross eyed */
}

void CFlaahgra::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {

  switch (msg) {
  case EScriptObjectMessage::InitializedInArea: {
    if (!x8e4_25_loading && !x8e4_24_loaded) {
      const_cast<CGameArea::CPostConstructed*>(mgr.WorldNC()->GetAreaAlways(GetAreaIdAlways())->GetPostConstructed())
          ->x113c_playerActorsLoading++;
      x8e4_25_loading = true;
    }

    GetMirrorWaypoints(mgr);
    break;
  }
  case EScriptObjectMessage::Activate: {
    GatherAssets(mgr);
    if (x8e5_25_)
      break;

    SetupCollisionManagers(mgr);
    x6d0_rendererId = mgr.AllocateUniqueId();
    mgr.AddObject(new CFlaahgraRenderer(x6d0_rendererId, GetUniqueId(), "Flaahgra Renderer"sv,
                                        CEntityInfo(GetAreaIdAlways(), NullConnectionList), GetTransform()));

    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Crouch);
    x450_bodyController->Activate(mgr);
    x8e5_27_ = true;
    break;
  }
  case EScriptObjectMessage::Deleted: {
    if (!x8e5_27_)
      break;

    x79c_leftArmCollision->Destroy(mgr);
    x7a0_rightArmCollision->Destroy(mgr);
    x7a4_sphereCollision->Destroy(mgr);
    mgr.FreeScriptObject(x6d0_rendererId);
    x6d0_rendererId = kInvalidUniqueId;
    x8e5_27_ = false;
    break;
  }
  case EScriptObjectMessage::Touched: {
    if (HealthInfo(mgr)->GetHP() <= 0.f)
      break;

    if (TCastToConstPtr<CCollisionActor> colAct = mgr.ObjectById(uid)) {
      if (colAct->GetLastTouchedObject() == mgr.GetPlayer().GetUniqueId() && x420_curDamageRemTime <= 0.f) {
        CDamageInfo contactDamage = GetContactDamage();
        if (x7a8_ == 4)
          contactDamage = x7dc_;

        if (sub801ae670())
          contactDamage.SetDamage(0.5f * contactDamage.GetDamage());

        if (x788_ == 2)
          contactDamage.SetDamage(1.33f * contactDamage.GetDamage());

        mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer().GetUniqueId(), GetUniqueId(), contactDamage,
                        CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), {});
        x420_curDamageRemTime = x424_damageWaitTime;
      }
    }
    break;
  }
  case EScriptObjectMessage::Damage: {
    if (HealthInfo(mgr)->GetHP() <= 0.f)
      break;

    if (!sub801aebb8(uid))
      break;

    if (TCastToConstPtr<CCollisionActor> colAct = mgr.GetObjectById(uid)) {
      if (TCastToConstPtr<CGameProjectile> proj = mgr.GetObjectById(colAct->GetLastTouchedObject())) {
        if (x780_ != 3)
          break;
        if (IsDizzy(mgr, 0.f) && !x450_bodyController->HasBodyState(pas::EAnimationState::LoopReaction)) {
          TakeDamage({}, 0.f);

          if ((x56c_.x140_ - proj->GetDamageInfo().GetDamage()) >= x810_) {
            x450_bodyController->GetCommandMgr().DeliverCmd(CBCLoopHitReactionCmd(pas::EReactionType::One));
          } else if (uid == x80c_headActor) {
            if (proj->GetDamageInfo().GetWeaponMode().IsCharged() ||
                proj->GetDamageInfo().GetWeaponMode().IsComboed() ||
                proj->GetDamageInfo().GetWeaponMode().GetType() == EWeaponType::Missile) {
              x450_bodyController->GetCommandMgr().DeliverCmd(CBCKnockBackCmd(-GetTranslation(), pas::ESeverity::One));
            }
          }
        } else {
          if (x8e5_30_)
            TakeDamage({}, 0.f);

          if (proj->GetDamageInfo().GetWeaponMode().IsCharged() || proj->GetDamageInfo().GetWeaponMode().IsComboed() ||
              proj->GetDamageInfo().GetWeaponMode().GetType() == EWeaponType::Missile) {
            x450_bodyController->GetCommandMgr().DeliverCmd(CBCAdditiveFlinchCmd(1.f));
          }
        }
      }
    }
    break;
  }
  case EScriptObjectMessage::Decrement: {
    x780_ = 0;
    break;
  }
  case EScriptObjectMessage::Close: {
    sub801ae980(mgr);
    break;
  }
  case EScriptObjectMessage::Start: {
    x8e4_31_ = false;
    break;
  }
  case EScriptObjectMessage::Stop: {
    x8e4_31_ = true;
    break;
  }
  case EScriptObjectMessage::Play: {
    x7d0_ = 3.f;
    x8e5_24_ = true;
    break;
  }
  case EScriptObjectMessage::Action: {
    if (TCastToConstPtr<CGameProjectile>(mgr.GetObjectById(uid)))
      x7f8_ = x788_;
    break;
  }
  case EScriptObjectMessage::SetToMax: {
    x7d4_ = 0.f;
    break;
  }
  case EScriptObjectMessage::Reset: {
    x8e5_28_ = true;
    break;
  }
  default:
    break;
  }
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
}

void CFlaahgra::Death(CStateManager& mgr, const zeus::CVector3f& dir, EScriptObjectState state) {
  if (!x400_25_alive)
    return;

  x330_stateMachineState.SetState(mgr, *this, GetStateMachine(), "Dead"sv);
  if (x450_bodyController->GetPercentageFrozen() > 0.f)
    x450_bodyController->UnFreeze();

  x400_25_alive = false;
}

void CFlaahgra::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  switch(type) {
  case EUserEventType::Projectile: {
    // TODO: Implement
    break;
  }
  case EUserEventType::BeginAction: {
    x8e4_26_ = true;
    x7c4_ = GetEndActionTime();
    break;
  }
  case EUserEventType::ScreenShake: {
    // TODO: Implement
    break;
  }
  case EUserEventType::AlignTargetRot: {
    if (x77c_ == kInvalidUniqueId)
      break;
    if (TCastToPtr<CScriptWaypoint> wp = mgr.ObjectById(x77c_)) {
      mgr.SendScriptMsg(wp, GetUniqueId(), EScriptObjectMessage::Arrived);
      if (x7f8_ > 0)
        --x7f8_;
    }
    break;
  }
  case EUserEventType::GenerateEnd: {
    // TODO: Implement
    break;
  }
  case EUserEventType::ObjectDrop: {
    SendScriptMsgs(EScriptObjectState::Modify, mgr, EScriptObjectMessage::None);
    break;
  }
  default: break;
  }

  CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}
void CFlaahgra::LoadDependencies(CAssetId dgrpId) {
  if (!dgrpId.IsValid()) {
    ResetModelDataAndBodyController();
    x8e4_24_loaded = true;
    return;
  }

  x8c8_depGroup = {g_SimplePool->GetObj({SBIG('DGRP'), dgrpId})};
  x8c8_depGroup->Lock();
}

void CFlaahgra::ResetModelDataAndBodyController() {
  SetModelData(std::make_unique<CModelData>(x8ac_));
  _CreateShadow();
  CreateShadow(true);
  x94_simpleShadow->SetAlwaysCalculateRadius(false);
  BuildBodyController(EBodyType::Restricted);
  x6cc_boneTracking.reset(new CBoneTracking(*GetModelData()->GetAnimationData(), "Head_1"sv, zeus::degToRad(80.f),
                                            zeus::degToRad(180.f), EBoneTrackingFlags::None));
}
void CFlaahgra::GatherAssets(CStateManager& mgr) {
  if (x8e4_24_loaded)
    return;

  x8c8_depGroup->GetObj();
  LoadTokens(mgr);

  if (x8e4_24_loaded)
    return;

  for (const CToken& tok : x8d4_tokens)
    tok.GetObj();

  FinalizeLoad(mgr);
}

void CFlaahgra::LoadTokens(CStateManager& mgr) {
  if (!x8d4_tokens.empty()) {
    for (const CToken& tok : x8d4_tokens) {
      if (!tok.IsLoaded())
        return;
    }

    FinalizeLoad(mgr);
  }

  if (!x8c8_depGroup)
    return;

  TToken<CDependencyGroup> depGroup = *x8c8_depGroup;
  if (depGroup->GetObjectTagVector().empty()) {
    FinalizeLoad(mgr);
    return;
  }

  if (x8d4_tokens.empty()) {
    x8d4_tokens.reserve(depGroup->GetObjectTagVector().size());

    for (const auto& tag : depGroup->GetObjectTagVector()) {
      CToken token = g_SimplePool->GetObj({tag.type, tag.id});
      token.Lock();
      x8d4_tokens.push_back(token);
    }
  }
}
void CFlaahgra::FinalizeLoad(CStateManager& mgr) {
  x8e4_24_loaded = true;
  if (x8e4_25_loading) {
    const_cast<CGameArea::CPostConstructed*>(mgr.WorldNC()->GetAreaAlways(GetAreaIdAlways())->GetPostConstructed())
        ->x113c_playerActorsLoading--;
    x8e4_25_loading = false;
  }

  ResetModelDataAndBodyController();
}

void CFlaahgra::Think(float dt, CStateManager& mgr) {
  if (!GetActive())
    return;

  CPatterned::Think(dt, mgr);
  x6cc_boneTracking->Update(dt);
  UpdateCollisionManagers(dt, mgr);
  x6cc_boneTracking->PreRender(mgr, *ModelData()->AnimationData(), GetTransform(), GetModelData()->GetScale(),
                               *x450_bodyController);
  UpdateSmallScaleReGrowth(dt);
  UpdateHealthInfo(mgr);
  UpdateAimPosition(mgr, dt);
  x15c_force = {};
  x168_impulse = {};
}

void CFlaahgra::PreThink(float dt, CStateManager& mgr) {
  if (!x8e4_24_loaded)
    LoadTokens(mgr);

  CPatterned::PreThink(dt, mgr);
}

void CFlaahgra::GetMirrorWaypoints(CStateManager& mgr) {
  x770_mirrorWaypoints.clear();

  for (const SConnection& conn : x20_conns) {
    if (conn.x0_state != EScriptObjectState::Modify || conn.x4_msg != EScriptObjectMessage::Follow)
      continue;
    TUniqueId uid = mgr.GetIdForScript(conn.x8_objId);
    if (TCastToConstPtr<CScriptWaypoint> wp = mgr.GetObjectById(uid))
      x770_mirrorWaypoints.push_back(uid);
  }
}

void CFlaahgra::AddCollisionList(const SJointInfo* joints, int count,
                                 std::vector<CJointCollisionDescription>& outJoints) {
  const CAnimData* animData = GetModelData()->GetAnimationData();
  for (u32 i = 0; i < count; ++i) {
    CSegId from = animData->GetLocatorSegId(joints[i].from);
    CSegId to = animData->GetLocatorSegId(joints[i].to);

    if (to != 0xFF && from != 0xFF) {
      outJoints.push_back(CJointCollisionDescription::SphereSubdivideCollision(
          to, from, joints[i].radius, joints[i].separation, CJointCollisionDescription::EOrientationType::One,
          joints[i].from, 10.f));
    }
  }
}

void CFlaahgra::AddSphereCollisionList(const SSphereJointInfo* joints, int count,
                                       std::vector<CJointCollisionDescription>& outJoints) {
  const CAnimData* animData = GetModelData()->GetAnimationData();
  for (u32 i = 0; i < count; ++i) {
    CSegId seg = animData->GetLocatorSegId(joints[i].name);
    if (seg != 0xFF) {
      outJoints.push_back(CJointCollisionDescription::SphereCollision(seg, joints[i].radius, joints[i].name, 10.f));
    }
  }
}

void CFlaahgra::SetupHealthInfo(CStateManager& mgr) {
  x7fc_sphereColliders.clear();

  for (u32 i = 0; i < x7a4_sphereCollision->GetNumCollisionActors(); ++i) {
    const auto& desc = x7a4_sphereCollision->GetCollisionDescFromIndex(i);
    TUniqueId uid = desc.GetCollisionActorId();
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(uid)) {
      *colAct->HealthInfo(mgr) = *HealthInfo(mgr);
      if (desc.GetName() == "Head_1"sv)
        x80c_headActor = uid;

      x7fc_sphereColliders.push_back(uid);
    }
  }

  x818_ = HealthInfo(mgr)->GetHP();
}

void CFlaahgra::SetupCollisionManagers(CStateManager& mgr) {
  std::vector<CJointCollisionDescription> leftArmjointList;
  zeus::CVector3f oldScale = GetModelData()->GetScale();
  leftArmjointList.reserve(3);
  AddCollisionList(skLeftArmJointList, 3, leftArmjointList);
  x79c_leftArmCollision.reset(
      new CCollisionActorManager(mgr, GetUniqueId(), GetAreaIdAlways(), leftArmjointList, true));
  SetMaterialProperties(x79c_leftArmCollision, mgr);
  std::vector<CJointCollisionDescription> rightArmJointList;
  rightArmJointList.reserve(3);
  AddCollisionList(skRightArmJointList, 3, rightArmJointList);
  x7a0_rightArmCollision.reset(
      new CCollisionActorManager(mgr, GetUniqueId(), GetAreaIdAlways(), rightArmJointList, true));
  SetMaterialProperties(x7a0_rightArmCollision, mgr);
  std::vector<CJointCollisionDescription> sphereJointList;
  sphereJointList.reserve(5);
  AddSphereCollisionList(skSphereJointList, 5, sphereJointList);
  x7a4_sphereCollision.reset(new CCollisionActorManager(mgr, GetUniqueId(), GetAreaIdAlways(), sphereJointList, true));
  SetMaterialProperties(x7a4_sphereCollision, mgr);
  SetupHealthInfo(mgr);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      {EMaterialTypes::Solid},
      {EMaterialTypes::CollisionActor, EMaterialTypes::AIPassthrough, EMaterialTypes::Player}));
  AddMaterial(EMaterialTypes::ProjectilePassthrough, EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
  RemoveMaterial(EMaterialTypes::Solid, mgr);
  ModelData()->SetScale(oldScale);
  x7a4_sphereCollision->AddMaterial(mgr, {EMaterialTypes::Unknown53, EMaterialTypes::CameraPassthrough});
  x79c_leftArmCollision->AddMaterial(mgr, {EMaterialTypes::Unknown53, EMaterialTypes::CameraPassthrough});
  x7a0_rightArmCollision->AddMaterial(mgr, {EMaterialTypes::Unknown53, EMaterialTypes::CameraPassthrough});
}

void CFlaahgra::sub801ae980(CStateManager& mgr) {
  HealthInfo(mgr)->SetHP(HealthInfo(mgr)->GetHP() - x56c_.x8_);
  x7d4_ = x56c_.xc_;
  x8e4_29_getup = true;
  x430_damageColor = skUnkColor;
  ++x788_;
}

void CFlaahgra::UpdateHeadDamageVulnerability(CStateManager& mgr, bool b) {
  if (TCastToPtr<CCollisionActor> head = mgr.ObjectById(x80c_headActor)) {
    head->SetDamageVulnerability(b ? *GetDamageVulnerability() : x56c_.x10_);
  }
}

void CFlaahgra::FadeIn(CStateManager& mgr, EStateMsg msg, float) {
  if (msg != EStateMsg::Activate)
    return;

  if (HealthInfo(mgr)->GetHP() > 0.f)
    SendScriptMsgs(EScriptObjectState::Exited, mgr, EScriptObjectMessage::None);

  if (!x8e4_29_getup)
    SendScriptMsgs(EScriptObjectState::CloseIn, mgr, EScriptObjectMessage::None);
}

void CFlaahgra::FadeOut(CStateManager& mgr, EStateMsg msg, float) {
  if (msg != EStateMsg::Activate)
    return;

  x7a4_sphereCollision->SetActive(mgr, true);
  x79c_leftArmCollision->SetActive(mgr, true);
  x7a0_rightArmCollision->SetActive(mgr, true);
  x81c_ = GetModelData()->GetScale().z();
  UpdateScale(1.f, x81c_, x56c_.x4_);
  x8e4_26_ = false;
  x7c0_ = 2.f;
  x780_ = 3;
  x8e4_29_getup = false;
  x430_damageColor = skDamageColor;
  x450_bodyController->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::NextState));
  x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
  x8e5_29_ = false;
}
void CFlaahgra::UpdateCollisionManagers(float dt, CStateManager& mgr) {
  x7a4_sphereCollision->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);
  x79c_leftArmCollision->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);
  x7a0_rightArmCollision->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);
}
void CFlaahgra::UpdateSmallScaleReGrowth(float dt) {
  if (x7c0_ > 0.f)
    x7c0_ -= (x788_ < 2 ? dt : 1.25f * dt);

  if (x7bc_ > 0.f)
    x7bc_ -= dt;

  if (x7d0_ > 0.f)
    x7d0_ -= dt;

  if (x7cc_ > 0.f)
    x7cc_ -= dt;

  if (!x8e4_29_getup || x7d8_ > 6.f)
    return;

  x430_damageColor = zeus::CColor::lerp(skUnkColor, zeus::skBlack, (M_PIF * std::cos(x7d8_)));
  TakeDamage({}, 0.f);
  x7d8_ += dt;
}

void CFlaahgra::UpdateHealthInfo(CStateManager& mgr) {
  float tmp = 0.f;
  for (const TUniqueId& uid : x7fc_sphereColliders) {
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(uid)) {
      CHealthInfo* inf = colAct->HealthInfo(mgr);

      tmp = zeus::max(tmp, x818_ - inf->GetHP());
    }
  }

  if (x780_ == 3) {
       if (!IsDizzy(mgr, 0.f))
         x814_ += tmp;
       else
         x810_ += tmp;
  } else {
    x814_ = 0.f;
    x810_ = 0.f;
  }

  CHealthInfo* hInfo = HealthInfo(mgr);
  if (hInfo->GetHP() <= 0.f) {
    Death(mgr, {}, EScriptObjectState::DeathRattle);
    RemoveMaterial(EMaterialTypes::Orbit, mgr);
    return;
  }
  for (const TUniqueId& uid : x7fc_sphereColliders) {
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(uid)) {
      colAct->HealthInfo(mgr)->SetHP(x818_);
    }
  }
}
void CFlaahgra::UpdateAimPosition(CStateManager& mgr, float dt) {
  if (TCastToConstPtr<CCollisionActor> head = mgr.GetObjectById(x80c_headActor)) {
    pas::EAnimationState animState = x450_bodyController->GetBodyStateInfo().GetCurrentStateId();
    if (animState == pas::EAnimationState::GroundHit || animState == pas::EAnimationState::LieOnGround)
      return;

    zeus::CVector3f vec;
    if (x780_ != 0 && !x8e4_28_ && !sub801e4f8())
      vec = head->GetTranslation();
    else
      vec = GetTranslation() + zeus::CVector3f(0.f, 0.f, 3.7674999f) +
            (zeus::CVector3f(0.f, 0.f, 4.1550002f) * GetModelData()->GetScale());

    zeus::CVector3f diff = vec - x820_;
    if (diff.canBeNormalized()) {
      if (diff.magnitude() > (125.f * dt))
        x820_ += ((1.f / diff.magnitude()) * diff) * (125.f * dt);
      else
        x820_ = vec;
    }
  }
}
void CFlaahgra::SetMaterialProperties(const std::unique_ptr<CCollisionActorManager>& actMgr, CStateManager& mgr) {
  for (u32 i = 0; i < actMgr->GetNumCollisionActors(); ++i) {
    TUniqueId uid = actMgr->GetCollisionDescFromIndex(i).GetCollisionActorId();
    if (CCollisionActor* colAct = static_cast<CCollisionActor*>(mgr.ObjectById(uid))) {
      colAct->SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
          {EMaterialTypes::Player},
          {EMaterialTypes::Trigger, EMaterialTypes::CollisionActor, EMaterialTypes::Immovable}));
      colAct->AddMaterial(EMaterialTypes::Trigger, EMaterialTypes::ScanPassthrough, mgr);
      colAct->SetDamageVulnerability(*GetDamageVulnerability());
    }
  }
}
bool CFlaahgra::ShouldTurn(CStateManager& mgr, float) {
  zeus::CVector2f posDiff = mgr.GetPlayer().GetTranslation().toVec2f() - GetTranslation().toVec2f();
  zeus::CVector2f frontVec = x34_transform.frontVector().toVec2f();
  return zeus::CVector2f::getAngleDiff(frontVec, posDiff) > zeus::degToRad(15.f);
}

void CFlaahgra::TurnAround(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x6cc_boneTracking->SetTarget(mgr.GetPlayer().GetUniqueId());
    x6cc_boneTracking->SetActive(true);
    x8e5_29_ = false;
  } else if (msg == EStateMsg::Update) {
    if (!ShouldTurn(mgr, 0.f))
      return;

    float dt = 0.f;
    if (GetModelData()->GetAnimationData()->GetSpeedScale() > 0.f)
      dt = 1.5f / GetModelData()->GetAnimationData()->GetSpeedScale();

    zeus::CVector3f offset = mgr.GetPlayer().GetAimPosition(mgr, dt) - GetTranslation();
    if (offset.canBeNormalized()) {
      x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd({}, offset.normalized(), 1.f));
    }
  } else if (msg == EStateMsg::Deactivate) {
    x6cc_boneTracking->SetActive(false);
  }
}
bool CFlaahgra::sub801ae670() { return (x7a8_ == 2 || x7a8_ == 3 || x7a8_ == 4); }
bool CFlaahgra::sub801aebb8(TUniqueId) { return false; }
void CFlaahgra::GetUp(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x568_ = 0;
    x784_ = x780_;
    x8e4_28_ = true;
  } else if (msg == EStateMsg::Update) {
    if (x568_ == 0) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Getup) {
        x568_ = 2;
        x7a8_ = (!x8e4_29_getup ? 4 : -1);
        SetCollisionActorBounds(mgr, x79c_leftArmCollision, skUnkVec1);
        SetCollisionActorBounds(mgr, x7a0_rightArmCollision, skUnkVec1);
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCGetupCmd(pas::EGetupType(x8e4_29_getup)));
      }
    } else if (x568_ == 2 && x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Getup) {
      x568_ = 4;
    }
  } else if (msg == EStateMsg::Deactivate) {
    x7c0_ = (x8e4_29_getup ? 5.f : 0.f);
    x7a8_ = -1;
    x8e4_28_ = false;
    x8e4_29_getup = false;
    SetCollisionActorBounds(mgr, x79c_leftArmCollision, {});
    SetCollisionActorBounds(mgr, x7a0_rightArmCollision, {});
    x430_damageColor = skDamageColor;
  }
}

void CFlaahgra::Growth(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x568_ = 0;
    x8e4_26_ = false;
    x450_bodyController->GetCommandMgr().DeliverCmd(CBCGenerateCmd(pas::EGenerateType::Two));
    x81c_ = GetModelData()->GetScale().z();
  } else if (msg == EStateMsg::Update) {
    if (x568_ == 0) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Generate)
        x568_ = 2;
      else
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCGenerateCmd(pas::EGenerateType::Two));
    } else if (x568_ == 2) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Generate) {
        x568_ = 4;
      } else if (x8e4_26_) {
        UpdateScale((x7c4_ > 0.f ? 1.f - (GetEndActionTime() / x7c4_) : 1.f), x81c_, x56c_.x4_);
      }

      x450_bodyController->GetCommandMgr().SetTargetVector(mgr.GetPlayer().GetTranslation() - GetTranslation());
    }
  } else if (msg == EStateMsg::Deactivate) {
    UpdateScale(1.f, x81c_, x56c_.x4_);
    x8e4_26_ = false;
    x780_ = 3;
    x79c_leftArmCollision->SetActive(mgr, true);
    x7a0_rightArmCollision->SetActive(mgr, true);
    x8e4_29_getup = false;
    x430_damageColor = skDamageColor;
  }
}


void CFlaahgra::SetCollisionActorBounds(CStateManager& mgr, const std::unique_ptr<CCollisionActorManager>& colMgr,
                                        const zeus::CVector3f& extendedBounds) {
  for (u32 i = 0; i < colMgr->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& jointDesc = colMgr->GetCollisionDescFromIndex(i);
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(jointDesc.GetCollisionActorId())) {
      colAct->SetExtendedTouchBounds(extendedBounds);
    }
  }
}
void CFlaahgra::UpdateScale(float t, float min, float max) {
  ModelData()->SetScale(zeus::skOne3f * (t * (max - min) + min));
}

float CFlaahgra::GetEndActionTime() {
  CCharAnimTime eventTime =
      GetModelData()->GetAnimationData()->GetTimeOfUserEvent(EUserEventType::EndAction, CCharAnimTime::Infinity());
  if (eventTime == CCharAnimTime::Infinity())
    return 0.f;

  return eventTime.GetSeconds();
}

void CFlaahgra::Generate(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x568_ = 0;
  } else if (msg == EStateMsg::Update) {
    if (x568_ == 0) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Generate) {
        x568_ = 2;
        x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCGenerateCmd(pas::EGenerateType::Zero));
      }
    } else if (x568_ == 2 &&
               x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Generate) {
      x568_ = 4;
    }
  } else if (msg == EStateMsg::Deactivate) {
    x7a4_sphereCollision->SetActive(mgr, true);
    x7c0_ = 11.f;
  }
}
zeus::CVector3f CFlaahgra::GetAttacktargetPos(CStateManager& mgr) const {
  if (mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed)
    return mgr.GetPlayer().GetMorphBall()->GetBallToWorld().origin;

  return mgr.GetPlayer().GetTranslation() + zeus::CVector3f(0.f, 0.f, -.5f + mgr.GetPlayer().GetEyeHeight());
}

CFlaahgraPlants::CFlaahgraPlants(const TToken<CGenDescription>& genDesc, const CActorParameters& actParms,
                                 TUniqueId uid, TAreaId aId, TUniqueId owner, const zeus::CTransform& xf,
                                 const CDamageInfo& dInfo, const zeus::CVector3f& extents)
: CActor(uid, true, "Flaahgra Plants"sv, CEntityInfo(aId, NullConnectionList), xf, CModelData::CModelDataNull(),
         CMaterialList(EMaterialTypes::Projectile), actParms, kInvalidUniqueId)
, xe8_elementGen(new CElementGen(genDesc))
, xf0_ownerId(owner)
, x130_obbox(xf, extents) {
  xe8_elementGen->SetGlobalOrientation(xf.getRotation());
  xe8_elementGen->SetGlobalTranslation(xf.origin);
  x110_aabox = {x130_obbox.calculateAABox(xf)};
}
void CFlaahgraPlants::Accept(IVisitor& visitor) {
  visitor.Visit(this);
}
void CFlaahgraPlants::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, uid, mgr);

  if (msg == EScriptObjectMessage::Registered) {
    xe8_elementGen->SetParticleEmission(true);
    if (x16c_colAct == kInvalidUniqueId) {
      x16c_colAct = mgr.AllocateUniqueId();
      CCollisionActor* colAct = new CCollisionActor(x16c_, GetAreaIdAlways(), GetUniqueId(),
                                                    x130_obbox.extents + zeus::CVector3f(10.f, 0.f, 10.f), {}, true,
                                                    0.001f, "Flaahgra Plants"sv);

      colAct->SetTransform(GetTransform());
      colAct->SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
          {EMaterialTypes::Player}, {EMaterialTypes::Trigger, EMaterialTypes::CollisionActor,
                                     EMaterialTypes::NoStaticCollision, EMaterialTypes::Character}));
      CMaterialList materialList = colAct->GetMaterialList();
      materialList.Add(EMaterialTypes::ProjectilePassthrough);
      materialList.Add(EMaterialTypes::Immovable);
      colAct->SetMaterialList(materialList);
      mgr.AddObject(colAct);
      mgr.SetActorAreaId(*colAct, GetAreaIdAlways());
    }
  } else if (msg == EScriptObjectMessage::Deleted && (x16c_colAct != kInvalidUniqueId) {
    mgr.FreeScriptObject(x16c_colAct);
  }
}
void CFlaahgraPlants::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    xe8_elementGen->Update(dt);
    x12c_lastDt = dt;
  }

  if (xe8_elementGen->IsSystemDeletable())
    mgr.FreeScriptObject(GetUniqueId());
}
void CFlaahgraPlants::AddToRenderer(const zeus::CFrustum& frustum, const CStateManager& mgr) const {
  g_Renderer->AddParticleGen(*xe8_elementGen.get());
  CActor::AddToRenderer(frustum, mgr);
}

void CFlaahgraPlants::Touch(CActor& act, CStateManager& mgr) {
  if (act.GetUniqueId() != mgr.GetPlayer().GetUniqueId() || !x110_aabox)
    return;

  zeus::COBBox plObb = zeus::COBBox::FromAABox(mgr.GetPlayer().GetBoundingBox(), {});

  if (x130_obbox.OBBIntersectsBox(plObb)) {
    /* TODO: Finish */
  }
}
} // namespace urde::MP1
