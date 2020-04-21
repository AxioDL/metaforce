#include "Runtime/MP1/World/CFlaahgra.hpp"

#include "Runtime/CDependencyGroup.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Camera/CCameraManager.hpp"
#include "Runtime/Camera/CFirstPersonCamera.hpp"
#include "Runtime/Character/CBoneTracking.hpp"
#include "Runtime/Collision/CCollisionActor.hpp"
#include "Runtime/Collision/CCollisionActorManager.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/MP1/World/CFlaahgraProjectile.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CGameArea.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CScriptWaypoint.hpp"
#include "Runtime/World/CWorld.hpp"
#include "Runtime/World/ScriptLoader.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {
constexpr zeus::CColor skDamageColor{0.5f, 0.5f, 0.f, 1.f};
constexpr zeus::CColor skUnkColor{0.5f, 0.f, 0.f, 1.f};
constexpr zeus::CVector3f skUnkVec1{0.5f, 7.f, 0.f};

constexpr std::array<SJointInfo, 3> skLeftArmJointList{{
    {"L_elbow", "L_blade", 0.6f, 1.f},
    {"L_blade", "L_claw", 0.6f, 1.f},
    {"L_CLAW_LCTR", "L_CLAW_END_LCTR", 0.6f, 1.f},
}};

constexpr std::array<SJointInfo, 3> skRightArmJointList{{
    {"R_elbow", "R_blade", 0.6f, 1.f},
    {"R_blade", "R_claw", 0.6f, 1.f},
    {"R_CLAW_LCTR", "R_CLAW_END_LCTR", 0.6f, 1.f},
}};

constexpr std::array<SSphereJointInfo, 5> skSphereJointList{{
    {"Head_1", 1.5f},
    {"Spine_2", 1.5f},
    {"Spine_4", 1.5f},
    {"Spine_6", 1.5f},
    {"Collar", 1.5f},
}};

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

void CFlaahgraRenderer::AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) {
  if (const CActor* act = static_cast<const CActor*>(mgr.GetObjectById(xe8_owner))) {
    if (act->HasModelData() && (act->GetModelData()->HasAnimData() || act->GetModelData()->HasNormalModel())) {
      act->GetModelData()->RenderParticles(frustum);
    }
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
, x7dc_(x404_contactDamage)
, x820_(xf.origin)
, x8a0_(xf.frontVector())
, x8ac_(animRes) {
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

  float curAngle = zeus::degToRad(17.5f);
  while (curAngle < zeus::degToRad(360.f)) {
    x82c_.push_back(GetTransform().rotate(zeus::CVector3f(std::cos(curAngle), std::sin(curAngle), 0.f)));
    x860_.push_back(GetTransform().rotate(
        zeus::CVector3f(std::cos(curAngle + zeus::degToRad(45.f)), std::sin(curAngle + zeus::degToRad(45.f)), 0.f)));

    curAngle += zeus::degToRad(90.f);
  }
}

void CFlaahgra::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {

  switch (msg) {
  case EScriptObjectMessage::InitializedInArea: {
    if (!x8e4_25_loading && !x8e4_24_loaded) {
      mgr.GetWorld()->GetArea(GetAreaIdAlways())->GetPostConstructed()->x113c_playerActorsLoading++;
      x8e4_25_loading = true;
    }

    GetMirrorWaypoints(mgr);
    break;
  }
  case EScriptObjectMessage::Activate: {
    GatherAssets(mgr);
    if (x8e5_27_)
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
        else if (!sub801ae670())
          contactDamage.SetDamage(0.5f * contactDamage.GetDamage());

        if (x788_ >= 2)
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

    if (!IsSphereCollider(uid))
      break;

    if (TCastToConstPtr<CCollisionActor> colAct = mgr.GetObjectById(uid)) {
      if (TCastToConstPtr<CGameProjectile> proj = mgr.GetObjectById(colAct->GetLastTouchedObject())) {
        if (x780_ != 3)
          break;
        if (!IsDizzy(mgr, 0.f) && x450_bodyController->HasBodyState(pas::EAnimationState::LoopReaction)) {
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

void CFlaahgra::AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) {
  if ((!GetModelData()->HasAnimData() && !GetModelData()->HasNormalModel()) || xe4_30_outOfFrustum) {
    return;
  }

  if (CanRenderUnsorted(mgr)) {
    Render(mgr);
  } else {
    EnsureRendered(mgr);
  }
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
  switch (type) {
  case EUserEventType::Projectile: {
    return;
  }
  case EUserEventType::BeginAction: {
//#ifndef NDEBUG
//    printf("BeginAction\n");
//#endif
    x8e4_26_ = true;
    x7c4_ = GetEndActionTime();
    break;
  }
  case EUserEventType::ScreenShake: {
    RattlePlayer(mgr, GetLctrTransform(node.GetLocatorName()).origin);
    return;
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
    zeus::CVector3f boneOrigin = GetLctrTransform(node.GetLocatorName()).origin;
    zeus::CVector3f aimPos = mgr.GetPlayer().GetAimPosition(mgr, 0.f);
    boneOrigin.z() = zeus::max<float>(aimPos.z(), boneOrigin.z());
    zeus::CTransform xf = GetTransform();
    xf.origin = boneOrigin;

    CRayCastResult res = mgr.RayStaticIntersection(xf.origin, zeus::skDown, 100.f,
                                                   CMaterialFilter::MakeInclude({EMaterialTypes::Floor}));

    if (res.IsValid()) {
      CFlaahgraPlants* plants = new CFlaahgraPlants(x6d4_, x56c_.xd8_, mgr.AllocateUniqueId(), GetAreaIdAlways(),
                                                    GetUniqueId(), xf, x56c_.xbc_, {5.f, 10.f, 5.f});
      mgr.AddObject(plants);
      mgr.SetActorAreaId(*plants, GetAreaIdAlways());
      x7cc_ = 8.f;
    }
    x8e4_27_ = true;
    break;
  }
  case EUserEventType::ObjectDrop: {
    SendScriptMsgs(EScriptObjectState::Modify, mgr, EScriptObjectMessage::None);
    break;
  }
  default:
    break;
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
  x6cc_boneTracking =
      std::make_unique<CBoneTracking>(*GetModelData()->GetAnimationData(), "Head_1"sv, zeus::degToRad(80.f),
                                      zeus::degToRad(180.f), EBoneTrackingFlags::None);
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
    mgr.GetWorld()->GetArea(GetAreaIdAlways())->GetPostConstructed()->x113c_playerActorsLoading--;
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
  x6cc_boneTracking->PreRender(mgr, *GetModelData()->GetAnimationData(), GetTransform(), GetModelData()->GetScale(),
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

void CFlaahgra::AddCollisionList(const SJointInfo* joints, size_t count,
                                 std::vector<CJointCollisionDescription>& outJoints) {
  const CAnimData* animData = GetModelData()->GetAnimationData();

  for (size_t i = 0; i < count; ++i) {
    const auto& joint = joints[i];
    const CSegId from = animData->GetLocatorSegId(joint.from);
    const CSegId to = animData->GetLocatorSegId(joint.to);

    if (to.IsInvalid() || from.IsInvalid()) {
      continue;
    }

    outJoints.push_back(CJointCollisionDescription::SphereSubdivideCollision(
        to, from, joint.radius, joint.separation, CJointCollisionDescription::EOrientationType::One, joint.from, 10.f));
  }
}

void CFlaahgra::AddSphereCollisionList(const SSphereJointInfo* joints, size_t count,
                                       std::vector<CJointCollisionDescription>& outJoints) {
  const CAnimData* animData = GetModelData()->GetAnimationData();

  for (size_t i = 0; i < count; ++i) {
    const auto& joint = joints[i];
    const CSegId seg = animData->GetLocatorSegId(joint.name);

    if (seg.IsInvalid()) {
      continue;
    }

    outJoints.push_back(CJointCollisionDescription::SphereCollision(seg, joint.radius, joint.name, 10.f));
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

  x818_curHp = HealthInfo(mgr)->GetHP();
}

void CFlaahgra::SetupCollisionManagers(CStateManager& mgr) {
  std::vector<CJointCollisionDescription> leftArmjointList;
  zeus::CVector3f oldScale = GetModelData()->GetScale();
  leftArmjointList.reserve(skLeftArmJointList.size());
  AddCollisionList(skLeftArmJointList.data(), skLeftArmJointList.size(), leftArmjointList);
  x79c_leftArmCollision =
      std::make_unique<CCollisionActorManager>(mgr, GetUniqueId(), GetAreaIdAlways(), leftArmjointList, true);
  SetMaterialProperties(x79c_leftArmCollision, mgr);
  std::vector<CJointCollisionDescription> rightArmJointList;
  rightArmJointList.reserve(skRightArmJointList.size());
  AddCollisionList(skRightArmJointList.data(), skRightArmJointList.size(), rightArmJointList);
  x7a0_rightArmCollision =
      std::make_unique<CCollisionActorManager>(mgr, GetUniqueId(), GetAreaIdAlways(), rightArmJointList, true);
  SetMaterialProperties(x7a0_rightArmCollision, mgr);
  std::vector<CJointCollisionDescription> sphereJointList;
  sphereJointList.reserve(skSphereJointList.size());
  AddSphereCollisionList(skSphereJointList.data(), skSphereJointList.size(), sphereJointList);
  x7a4_sphereCollision =
      std::make_unique<CCollisionActorManager>(mgr, GetUniqueId(), GetAreaIdAlways(), sphereJointList, true);
  SetMaterialProperties(x7a4_sphereCollision, mgr);
  SetupHealthInfo(mgr);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      {EMaterialTypes::Solid},
      {EMaterialTypes::CollisionActor, EMaterialTypes::AIPassthrough, EMaterialTypes::Player}));
  AddMaterial(EMaterialTypes::ProjectilePassthrough, EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
  RemoveMaterial(EMaterialTypes::Solid, mgr);
  GetModelData()->SetScale(oldScale);
  x7a4_sphereCollision->AddMaterial(mgr, {EMaterialTypes::AIJoint, EMaterialTypes::CameraPassthrough});
  x79c_leftArmCollision->AddMaterial(mgr, {EMaterialTypes::AIJoint, EMaterialTypes::CameraPassthrough});
  x7a0_rightArmCollision->AddMaterial(mgr, {EMaterialTypes::AIJoint, EMaterialTypes::CameraPassthrough});
}

void CFlaahgra::sub801ae980(CStateManager& mgr) {
  HealthInfo(mgr)->SetHP(HealthInfo(mgr)->GetHP() - x56c_.x8_);
  x7d4_ = x56c_.xc_;
  x8e4_29_getup = true;
  x7d8_ = 0.f;
  x430_damageColor = skUnkColor;
  ++x788_;
}

void CFlaahgra::sub801ade80() {
  x894_ = (x7ac_ == 0 ? -GetTransform().basis[0] : GetTransform().basis[0]);

  const rstl::reserved_vector<zeus::CVector3f, 4>& vec = (x7ac_ == 0 ? x860_ : x82c_);

  float curDist = FLT_MIN;
  for (const zeus::CVector3f& v : vec) {
    if (v.magSquared() < 0.f)
      continue;

    float dist = GetTransform().basis[1].dot(v);
    if (dist > curDist) {
      curDist = dist;
      x894_ = v;
    }
  }
}

bool CFlaahgra::ShouldAttack(CStateManager& mgr, float) {
  if (x788_ <= 0 || x788_ > 3 || x7c0_ > 0.f || mgr.GetPlayer().IsInWaterMovement() || x8e4_31_)
    return false;

  CPlayer& player = mgr.GetPlayer();
  zeus::CVector2f diff = player.GetTranslation().toVec2f() - GetTranslation().toVec2f();

  float dist = diff.magSquared();

  float minSq = x2fc_minAttackRange * x2fc_minAttackRange;
  float maxSq = x300_maxAttackRange * x300_maxAttackRange;

  if ((player.GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed || dist < minSq ||
       dist > maxSq) &&
      (x7cc_ > 0.f || player.GetVelocity().magSquared() < 25.f))
    return false;

  return zeus::CVector2f::getAngleDiff(GetTransform().basis[1].toVec2f(), diff) < zeus::degToRad(45.f);
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
  x784_ = x780_;
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

  x430_damageColor = zeus::CColor::lerp(skUnkColor, zeus::skBlack, std::round(std::fabs(M_PIF * std::cos(x7d8_))));
  TakeDamage({}, 0.f);
  x7d8_ += dt;
}

void CFlaahgra::UpdateHealthInfo(CStateManager& mgr) {
  float tmp = 0.f;
  for (const TUniqueId& uid : x7fc_sphereColliders) {
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(uid)) {
      CHealthInfo* inf = colAct->HealthInfo(mgr);

      tmp = zeus::max(tmp, x818_curHp - inf->GetHP());
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
      colAct->HealthInfo(mgr)->SetHP(x818_curHp);
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

bool CFlaahgra::IsSphereCollider(TUniqueId uid) const {
  const auto it = std::find(x7fc_sphereColliders.cbegin(), x7fc_sphereColliders.cend(), uid);
  return it != x7fc_sphereColliders.end();
}

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
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCGetupCmd(pas::EGetupType(!x8e4_29_getup)));
      }
    } else if (x568_ == 2 &&
               x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Getup) {
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

      x450_bodyController->GetCommandMgr().DeliverTargetVector(mgr.GetPlayer().GetTranslation() - GetTranslation());
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
  float scale = (t * (max - min) + min);
  GetModelData()->SetScale(zeus::skOne3f * scale);
}

float CFlaahgra::GetEndActionTime() const {
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
zeus::CVector3f CFlaahgra::GetAttacktargetPos(const CStateManager& mgr) const {
  if (mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed)
    return mgr.GetPlayer().GetMorphBall()->GetBallToWorld().origin;

  return mgr.GetPlayer().GetTranslation() + zeus::CVector3f(0.f, 0.f, -.5f + mgr.GetPlayer().GetEyeHeight());
}

void CFlaahgra::RattlePlayer(CStateManager& mgr, const zeus::CVector3f& vec) {
  CPlayer& player = mgr.GetPlayer();
  /*
   zeus::CVector3f direction = vec - mgr.GetPlayer().GetTranslation();
   float dir = direction.magnitude(); Unused
  */

  if (player.GetSurfaceRestraint() == CPlayer::ESurfaceRestraints::Air || player.IsInWaterMovement())
    return;

  if (player.GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed) {
    if (mgr.GetCameraManager()->GetFirstPersonCamera()->GetUniqueId() != mgr.GetCameraManager()->GetCurrentCameraId())
      return;

    mgr.GetCameraManager()->AddCameraShaker(CCameraShakeData(2.f, 0.75f), true);
  } else {
    player.ApplyImpulseWR((mgr.GetPlayer().GetMass() * 0.75f) * (25.f * zeus::skUp), {});
    player.SetMoveState(CPlayer::EPlayerMovementState::ApplyJump, mgr);
  }
}

void CFlaahgra::Faint(CStateManager& mgr, EStateMsg msg, float arg) {
  static constexpr std::array kSeverities{pas::ESeverity::Zero, pas::ESeverity::One};

  if (msg == EStateMsg::Activate) {
    x568_ = 0;
    x7d4_ = 0.f;
    x8e5_24_ = false;
    x7f8_ = 0;
    SendScriptMsgs(EScriptObjectState::Entered, mgr, EScriptObjectMessage::None);
    SendScriptMsgs(EScriptObjectState::Retreat, mgr, EScriptObjectMessage::None);
    x450_bodyController->GetCommandMgr().DeliverCmd(
        CBCKnockDownCmd(-GetTransform().frontVector(), kSeverities[size_t(x7ac_)]));
  } else if (msg == EStateMsg::Update) {
    if (x568_ == 0) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Fall) {
        x568_ = 2;
        sub801ade80();
        UpdateHeadDamageVulnerability(mgr, true);
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(
            CBCKnockDownCmd(-GetTransform().frontVector(), kSeverities[size_t(x7ac_)]));
      }
    } else if (x568_ == 2) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::LieOnGround) {
        x7d4_ += arg;
        if (x7d4_ >= x56c_.xc_)
          x568_ = 4;
      } else {
        x450_bodyController->FaceDirection(x894_, arg);
      }
    }
  } else if (msg == EStateMsg::Deactivate) {
    x8e4_27_ = false;
    x7ac_ = !x7ac_;
    x780_ = 3;
    UpdateHeadDamageVulnerability(mgr, false);
  }
}

void CFlaahgra::Dead(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x568_ = (x450_bodyController->GetFallState() != pas::EFallState::Zero ||
             x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Fall);
    SendScriptMsgs(EScriptObjectState::CloseIn, mgr, EScriptObjectMessage::None);
  } else if (msg == EStateMsg::Update) {
    if (x568_ == 0) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Getup)
        return;

      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Fall) {
        x450_bodyController->GetCommandMgr().DeliverCmd(
            CBCKnockDownCmd(-GetTransform().frontVector(), pas::ESeverity::Two));
      } else {
        x568_ = 2;
        zeus::CTransform xf = zeus::lookAt(GetTranslation(), GetTranslation() + x8a0_, zeus::skUp);
        xf.origin = GetTranslation();
        SetTransform(xf);
        SendScriptMsgs(EScriptObjectState::Dead, mgr, EScriptObjectMessage::None);
        x8e5_26_ = true;
      }
    } else if (x568_ == 1) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Getup) {
        x568_ = 0;
        return;
      }

      x450_bodyController->GetCommandMgr().DeliverCmd(CBCGetupCmd(pas::EGetupType::Two));
    } else if (x568_ == 2 && x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Fall)
      mgr.FreeScriptObject(GetUniqueId());
  }
}

void CFlaahgra::Attack(CStateManager& mgr, EStateMsg msg, float arg) {
  static constexpr std::array kStates1{
      pas::ESeverity::Invalid, pas::ESeverity::Invalid, pas::ESeverity::Invalid,
      pas::ESeverity::Two,     pas::ESeverity::Invalid,
  };
  static constexpr std::array kSeverity{
      pas::ESeverity::Three, pas::ESeverity::Four, pas::ESeverity::One, pas::ESeverity::Zero, pas::ESeverity::Invalid,
  };

  if (msg == EStateMsg::Activate) {
    x568_ = 0;
    x7a8_ = sub801ae828(mgr);
    SendScriptMsgs(EScriptObjectState::Attack, mgr, EScriptObjectMessage::None);
  } else if (msg == EStateMsg::Update) {
    if (x568_ == 0) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::MeleeAttack) {
        x568_ = kStates1[x7a8_] != pas::ESeverity::Invalid ? 1 : 2;

        if (sub801ae670()) {
          SetCollisionActorBounds(mgr, x79c_leftArmCollision, skUnkVec1);
          SetCollisionActorBounds(mgr, x7a0_rightArmCollision, skUnkVec1);
        }

        x798_animState = x450_bodyController->GetBodyStateInfo().GetCurrentStateId();
        x78c_ = sub801ae754(mgr);
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCMeleeAttackCmd(kSeverity[x7a8_]));
      }
    } else if (x568_ == 1) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::MeleeAttack)
        x568_ = 4;
      else {
        if (x798_animState != x450_bodyController->GetBodyStateInfo().GetCurrentStateId()) {
          x568_ = 2;
        } else {
          if (!ShouldAttack(mgr, 0.f))
            return;

          x450_bodyController->GetCommandMgr().DeliverCmd(CBCMeleeAttackCmd(kStates1[x7a8_]));
        }
      }
    } else if (x568_ == 2) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::MeleeAttack)
        x568_ = 4;
      else
        x450_bodyController->GetCommandMgr().DeliverTargetVector(x78c_);
    }
  } else if (msg == EStateMsg::Deactivate) {
    SetCollisionActorBounds(mgr, x79c_leftArmCollision, {});
    SetCollisionActorBounds(mgr, x7a0_rightArmCollision, {});

    if (sub801ae670())
      x7c0_ = (1.f + x788_) / (x308_attackTimeVariation * mgr.GetActiveRandom()->Float() + x304_averageAttackTime);

    x7a8_ = -1;

    if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::MeleeAttack) {
      x450_bodyController->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::NextState));
    }
  }
}

u32 CFlaahgra::sub801ae828(const CStateManager& mgr) const {
  const CPlayer& player = mgr.GetPlayer();
  if (player.GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed) {
    if (x7cc_ > 0.f || player.GetVelocity().magSquared() < 25.f)
      return 3;
  }

  if (GetTransform().basis[0].dot(player.GetVelocity()) > 0.f)
    return 1;

  return 0;
}
zeus::CVector3f CFlaahgra::sub801ae754(const CStateManager& mgr) const {
  float dt = (sub801ae650() && mgr.GetPlayer().GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed
                  ? 0.75f
                  : 0.5f);
  return GetAimPosition(mgr, dt * x450_bodyController->GetAnimTimeRemaining()) - GetTranslation();
}

void CFlaahgra::Dizzy(CStateManager&, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x7b8_ = 0.f;
    x814_ = 0.f;
    x8e5_30_ = false;
  } else if (msg == EStateMsg::Update) {
    x7b8_ += arg;
    if (x7b8_ >= (x788_ < 2 ? x56c_.x144_ : -1.5f + x56c_.x144_)) {
      x450_bodyController->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::ExitState));
      x8e5_30_ = true;
    } else {
      x814_ = 0.f;
    }
  } else if (msg == EStateMsg::Deactivate) {
    x8e5_30_ = false;
    x810_ = x814_;
    x7bc_ = x56c_.x148_;
    x450_bodyController->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::ExitState));
  }
}
void CFlaahgra::Suck(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x568_ = 0;
    x8e4_26_ = false;
    x450_bodyController->GetCommandMgr().DeliverCmd(CBCGenerateCmd(pas::EGenerateType::Two));
    x784_ = x780_;
  } else if (msg == EStateMsg::Update) {
    if (x568_ == 0) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Getup)
        x568_ = 2;
      else
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCGetupCmd(pas::EGetupType::Zero));
    } else if (x568_ == 2) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Getup)
        x568_ = 4;
      else if (x8e4_26_) {
        UpdateScale(x7c4_ > 0.0f ? 1.f - (GetEndActionTime() / x7c4_) : 1.f, x56c_.x4_, x56c_.x0_);
      }
    }

  } else if (msg == EStateMsg::Deactivate) {
    x8e4_26_ = false;
    x780_ = 2;
    x79c_leftArmCollision->SetActive(mgr, false);
    x7a0_rightArmCollision->SetActive(mgr, false);
  }
}
void CFlaahgra::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x568_ = 0;
    SendScriptMsgs(EScriptObjectState::Attack, mgr, EScriptObjectMessage::None);
  } else if (msg == EStateMsg::Update) {
    if (x568_ == 0) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::ProjectileAttack) {
        x568_ = 2;
      } else {
        x7b4_ = 3;
        pas::ESeverity severity;
        if (x8e4_31_) {
          x7b4_ = 2;
          severity = pas::ESeverity::Six;
        } else if (mgr.GetPlayer().GetVelocity().magSquared() > 100.f) {
          if (mgr.GetPlayer().GetTransform().basis[0].magSquared() < 0.f) {
            x7b4_ = 1;
            severity = pas::ESeverity::Four;
            x72c_ = 0;
          } else {
            severity = pas::ESeverity::Three;
            x7b4_ = x72c_ = 0;
          }
        } else {
          severity = pas::ESeverity::Seven;
        }

        x450_bodyController->GetCommandMgr().DeliverCmd(
            CBCProjectileAttackCmd(severity, mgr.GetPlayer().GetTranslation(), false));
      }
    } else if (x568_ == 2) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::ProjectileAttack) {
        x568_ = 4;
      } else {
        x450_bodyController->GetCommandMgr().DeliverTargetVector(mgr.GetPlayer().GetTranslation() - GetTranslation());
      }
    }
  }
}
void CFlaahgra::Cover(CStateManager& mgr, EStateMsg msg, float) {
  static pas::ESeverity severities[2]{pas::ESeverity::Eight, pas::ESeverity::Seven};
  if (msg == EStateMsg::Activate) {
    x77c_ = GetMirrorNearestPlayer(mgr);
    x568_ = (x77c_ == kInvalidUniqueId ? 4 : 1);
    x6cc_boneTracking->SetTarget(mgr.GetPlayer().GetUniqueId());
    x6cc_boneTracking->SetActive(true);
  } else if (msg == EStateMsg::Update) {
    if (x568_ == 0) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::MeleeAttack)
        x568_ = 2;
      else
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCMeleeAttackCmd(severities[x7b0_]));
    } else if (x568_ == 1) {
      if (TCastToConstPtr<CActor> wp = mgr.GetObjectById(x77c_)) {
        zeus::CVector3f direction = wp->GetTranslation() - GetTranslation();
        if (zeus::CVector2f::getAngleDiff(GetTransform().basis[1].toVec2f(), direction.toVec2f()) >
                zeus::degToRad(15.f) &&
            direction.canBeNormalized()) {
          x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd({}, direction.normalized(), 1.f));
        } else {
          x568_ = 0;
          x6cc_boneTracking->SetActive(false);
        }
      } else
        x568_ = 4;
    } else if (x568_ == 2) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::MeleeAttack)
        x568_ = 4;
      else if (TCastToConstPtr<CActor> wp = mgr.GetObjectById(x77c_)) {
        x450_bodyController->GetCommandMgr().DeliverTargetVector(wp->GetTranslation() - GetTranslation());
      }
    }
  } else if (msg == EStateMsg::Deactivate) {
    if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::MeleeAttack) {
      x450_bodyController->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::NextState));
    }

    x77c_ = kInvalidUniqueId;
    x7bc_ = x56c_.x148_;
    x7b0_ ^= 1;
  }
}
void CFlaahgra::SpecialAttack(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x568_ = 0;
    x8e5_24_ = false;
    x7b4_ = 3;
    SendScriptMsgs(EScriptObjectState::Attack, mgr, EScriptObjectMessage::None);
  } else if (msg == EStateMsg::Update) {
    if (x568_ == 0) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::ProjectileAttack) {
        x568_ = 2;
        x8e4_30_ = true;
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(
            CBCProjectileAttackCmd(pas::ESeverity::Eight, mgr.GetPlayer().GetTranslation(), false));
      }
    } else if (x568_ == 2) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::ProjectileAttack) {}
    }
  }
}
bool CFlaahgra::CoverCheck(CStateManager& mgr, float) {
  if (x7f8_ <= 0 && x7bc_ > 0.f)
    return false;

  for (TUniqueId id : x770_mirrorWaypoints) {
    if (const CEntity* ent = mgr.GetObjectById(id)) {
      if (ent->GetActive())
        return true;
    }
  }

  return false;
}

TUniqueId CFlaahgra::GetMirrorNearestPlayer(const CStateManager& mgr) const {
  zeus::CVector3f playerPos = mgr.GetPlayer().GetTranslation();

  TUniqueId nearId = kInvalidUniqueId;
  float prevMag = -1.f;
  for (TUniqueId id : x770_mirrorWaypoints) {
    if (TCastToConstPtr<CActor> wp = mgr.GetObjectById(id)) {
      if (!wp->GetActive())
        continue;
      const float mag = (wp->GetTranslation() - playerPos).magSquared();
      if (mag > prevMag) {
        nearId = id;
        prevMag = mag;
      }
    }
  }

  return nearId;
}

void CFlaahgra::Enraged(CStateManager&, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x568_ = 0;
  } else if (msg == EStateMsg::Update) {
    if (x568_ == 0) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Taunt)
        x568_ = 2;
      else
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCTauntCmd(pas::ETauntType::Zero));
    } else if (x568_ == 2 && x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Taunt)
      x568_ = 4;
  } else if (msg == EStateMsg::Deactivate) {
    x7d0_ = 0.f;
  }
}

CFlaahgraPlants::CFlaahgraPlants(const TToken<CGenDescription>& genDesc, const CActorParameters& actParms,
                                 TUniqueId uid, TAreaId aId, TUniqueId owner, const zeus::CTransform& xf,
                                 const CDamageInfo& dInfo, const zeus::CVector3f& extents)
: CActor(uid, true, "Flaahgra Plants"sv, CEntityInfo(aId, NullConnectionList), xf, CModelData::CModelDataNull(),
         CMaterialList(EMaterialTypes::Projectile), actParms, kInvalidUniqueId)
, xe8_elementGen(std::make_unique<CElementGen>(genDesc))
, xf0_ownerId(owner)
, x130_obbox(xf, extents) {
  xe8_elementGen->SetOrientation(xf.getRotation());
  xe8_elementGen->SetTranslation(xf.origin);
  xe8_elementGen->SetModelsUseLights(true);
  x110_aabox = {x130_obbox.calculateAABox(xf)};
}

void CFlaahgraPlants::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CFlaahgraPlants::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, uid, mgr);

  if (msg == EScriptObjectMessage::Registered) {
    xe8_elementGen->SetParticleEmission(true);
    if (x16c_colAct == kInvalidUniqueId) {
      x16c_colAct = mgr.AllocateUniqueId();
      CCollisionActor* colAct = new CCollisionActor(x16c_colAct, GetAreaIdAlways(), GetUniqueId(),
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
  } else if (msg == EScriptObjectMessage::Deleted && x16c_colAct != kInvalidUniqueId) {
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

void CFlaahgraPlants::AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) {
  g_Renderer->AddParticleGen(*xe8_elementGen.get());
  CActor::AddToRenderer(frustum, mgr);
}

void CFlaahgraPlants::Touch(CActor& act, CStateManager& mgr) {
  if (act.GetUniqueId() != mgr.GetPlayer().GetUniqueId() || !x110_aabox)
    return;

  zeus::COBBox plObb = zeus::COBBox::FromAABox(mgr.GetPlayer().GetBoundingBox(), {});

  if (!x130_obbox.OBBIntersectsBox(plObb))
    return;

  CDamageInfo dInfo = xf4_damageInfo;
  float newDamage = x12c_lastDt * xf4_damageInfo.GetDamage();
  dInfo.SetDamage(newDamage);
  dInfo.SetRadiusDamage(newDamage);
  dInfo.SetNoImmunity(true);

  zeus::CVector3f diffVec = mgr.GetPlayer().GetTranslation() - GetTranslation();
  mgr.ApplyDamage(GetUniqueId(), act.GetUniqueId(), GetUniqueId(), dInfo,
                  CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}),
                  diffVec.dot(GetTransform().basis[0]) > 0.f ? GetTransform().basis[0] : -GetTransform().basis[0]);
}
} // namespace urde::MP1
