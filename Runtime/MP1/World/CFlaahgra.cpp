#include "CFlaahgra.hpp"
#include "CDependencyGroup.hpp"
#include "Character/CBoneTracking.hpp"
#include "Collision/CCollisionActor.hpp"
#include "Collision/CCollisionActorManager.hpp"
#include "MP1/World/CFlaahgraProjectile.hpp"
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

    //x450_bodyController->SetLocomotionType(pas::ELocomotionType::Crouch);
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
    break;
  }
  case EScriptObjectMessage::Damage: {
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

void CFlaahgra::SetupCollisionManagers(CStateManager& mgr) {
  std::vector<CJointCollisionDescription> leftArmjointList;
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
  sphereJointList.reserve(3);
  AddSphereCollisionList(skSphereJointList, 5, sphereJointList);
  x7a4_sphereCollision.reset(new CCollisionActorManager(mgr, GetUniqueId(), GetAreaIdAlways(), sphereJointList, true));
  SetMaterialProperties(x7a4_sphereCollision, mgr);
}

void CFlaahgra::sub801ae980(CStateManager& mgr) {
  HealthInfo(mgr)->SetHP(HealthInfo(mgr)->GetHP() - x56c_.x8_);
  x7d4_ = x56c_.xc_;
  x8e4_29_ = true;
  x430_damageColor = skUnkColor;
  ++x788_;
}

void CFlaahgra::FadeIn(CStateManager& mgr, EStateMsg msg, float) {
  if (msg != EStateMsg::Activate)
    return;

  if (HealthInfo(mgr)->GetHP() > 0.f)
    SendScriptMsgs(EScriptObjectState::Exited, mgr, EScriptObjectMessage::None);

  if (!x8e4_29_)
    SendScriptMsgs(EScriptObjectState::CloseIn, mgr, EScriptObjectMessage::None);
}

void CFlaahgra::FadeOut(CStateManager& mgr, EStateMsg msg, float) {
  if (msg != EStateMsg::Activate)
    return;

  x7a4_sphereCollision->SetActive(mgr, true);
  x79c_leftArmCollision->SetActive(mgr, true);
  x7a0_rightArmCollision->SetActive(mgr, true);
}
void CFlaahgra::UpdateCollisionManagers(float dt, CStateManager& mgr) {
  x7a4_sphereCollision->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);
  x79c_leftArmCollision->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);
  x7a0_rightArmCollision->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);
}
void CFlaahgra::UpdateSmallScaleReGrowth(float) {}
void CFlaahgra::UpdateHealthInfo(CStateManager&) {}
void CFlaahgra::UpdateAimPosition(CStateManager&, float) {}
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
} // namespace urde::MP1
