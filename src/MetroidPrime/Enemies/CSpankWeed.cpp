#include "MetroidPrime/Enemies/CSpankWeed.hpp"

#include "Kyoto/Animation/CPASAnimState.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"

// These strings survive from the unused collision actor implementation.
static const char* skJointNameList[] = {
    "Arm_2", "Arm_3",  "Arm_4",  "Arm_5",  "Arm_6",   "Arm_7",       "Arm_8",
    "Arm_9", "Arm_10", "Arm_11", "Arm_12", "Arm_end", "swoosh_LCTR", "lockon_target_LCTR",
    "Eye",
};

static const char* const skCollisionDebugStrings[] = {
    "??(??)",
    "WARNING: Non-uniform scale (%.2f, %.2f, %.2f) applied to Spank Weed...changing scale "
    "to (%.2f, %.2f, %.2f)\n",
    "Spank Weed Collision "};

const SSphereJointInfo CSpankWeed::skSphereJointInfoList[] = {
    {"Arm_4", 1.5f}, {"Arm_6", 1.f},  {"Arm_7", 1.f},        {"Arm_8", 1.f},
    {"Arm_9", 1.f},  {"Arm_11", 1.f}, {"swoosh_LCTR", 1.5f},
};

CSpankWeed::CSpankWeed(const TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                       const CTransform4f& xf, const CModelData& mData,
                       const CActorParameters& actParms, const CPatternedInfo& pInfo,
                       const float maxDetectionRange, const float maxHearingRange,
                       const float maxSightRange, const float hideTime)
: CPatterned(kC_SpankWeed, uid, name, kFT_Zero, info, xf, mData, pInfo, kMT_Flyer, kCT_One,
             kBT_Restricted, actParms, kCS_Medium)
, x568_maxDetectionRange(maxDetectionRange)
, x56c_detectionHeightRange(pInfo.GetDetectionHeightRange())
, x570_maxHearingRange(maxHearingRange)
, x574_maxSightRange(maxSightRange)
, x578_hideTime(hideTime)
, x57c_canKnockBack(false)
, x580_(0.f)
, x584_retreatOrigin(xf.GetTranslation())
, x590_(kInvalidUniqueId)
, x598_isHiding(true)
, x59c_lockonOffset(CVector3f::Zero())
, x5a8_lockonTarget(CVector3f::Zero())
, x5b4_state(-1)
, x5b8_previousState(-1)
, x5bc_animPhase(-1) {
  SetCallTouch(false);
  SetDrawShadow(false);

  const CVector3f modelScale = CVector3f(GetModelData()->GetScale());
  if (modelScale.GetX() != modelScale.GetY() || modelScale.GetX() != modelScale.GetZ()) {
    const float mag = modelScale.Magnitude();
    const float scale = mag / CMath::SqrtF(3.f);
    ModelData()->SetScale(CVector3f(scale, scale, scale));

    char buf[1024];
    sprintf(buf,
            "WARNING: Non-uniform scale (%.2f, %.2f, %.2f) applied to Spank Weed...changing scale "
            "to (%.2f, %.2f, %.2f)\n",
            modelScale.GetX(), modelScale.GetY(), modelScale.GetZ(), scale, scale, scale);
  }

  CMaterialList list = GetMaterialFilter().GetExcludeList();
  list.Add(CMaterialList(kMT_Character, kMT_Player));
  SetMaterialFilter(
      CMaterialFilter::MakeIncludeExclude(GetMaterialFilter().GetIncludeList(), list));

  const CSegId segId = GetAnimationData()->GetLocatorSegId(rstl::string_l("lockon_target_LCTR"));

  if (segId.val() != 0xFFu) {
    const CTransform4f locatorXf = GetAnimationData()->GetLocatorTransform(segId, nullptr);
    const CVector3f& scale = CVector3f(GetModelData()->GetScale());
    const CTransform4f scaledXf = GetTransform() * (CTransform4f::Scale(scale) * locatorXf);
    x5a8_lockonTarget = scaledXf.GetTranslation();
    x59c_lockonOffset = scaledXf.GetTranslation() - GetTranslation();
  }

  KnockBackCtrl().SetAutoResetImpulse(false);
}

ENTITY_ACCEPT_IMPL(CSpankWeed)

void CSpankWeed::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  bool oldActive = GetActive();
  switch (msg) {
  case kSM_Registered: {
    if (!x450_bodyController->GetIsActive()) {
      x450_bodyController->Activate(mgr);
      const CAABox box = GetBoundingBox();
      const float halfWidth = 0.5f * box.GetWidth();
      const float halfDepth = 0.5f * box.GetDepth();
      const float halfHeight = 0.5f * box.GetHeight();
      SetBoundingBox(CAABox(-halfWidth, -halfHeight, -halfDepth, halfWidth, halfHeight, halfDepth));
    }
    {
      rstl::vector< CJointCollisionDescription > joints;
      joints.reserve(12);
      for (uint i = 0; i < ARRAY_SIZE(skSphereJointInfoList); ++i) {
        const SSphereJointInfo& joint = skSphereJointInfoList[i];
        const CSegId id = GetAnimationData()->GetLocatorSegId(rstl::string_l(joint.name));
        if (id.val() != 0xff) {
          const CJointCollisionDescription desc = CJointCollisionDescription::SphereCollision(
              id, joint.radius, rstl::string_l(joint.name), 0.001f);
          joints.push_back(desc);
        }
      }
      x594_collisionMgr = rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(),
                                                        joints, GetActive());
      CMaterialList list;
      list.Add(CMaterialList(kMT_CameraPassthrough));
      list.Add(CMaterialList(kMT_Immovable));
      x594_collisionMgr->AddMaterial(mgr, list);
    }
    if (HasActorLights())
      ActorLights()->SetNeedsRelight(true);
    const CVector3f bias =
        GetTransform().BuildMatrix3f() * GetScaledLocatorTransform("swoosh_LCTR").GetTranslation();
    ActorLights()->SetActorPositionBias(bias);
    break;
  }
  case kSM_Touched: {
    CEntity* obj = mgr.ObjectById(uid);
    if (CCollisionActor* colAct = TCastToPtr< CCollisionActor >(obj)) {
      const TUniqueId touchedId = colAct->GetLastTouchedObject();
      CEntity* playerObj = mgr.ObjectById(touchedId);
      if (CPlayer* player = TCastToPtr< CPlayer >(playerObj)) {
        if (x420_curDamageRemTime <= 0.f && x5b4_state != 4 && x5b4_state != 6) {
          mgr.ApplyDamage(
              GetUniqueId(), player->GetUniqueId(), GetUniqueId(), GetContactDamage(),
              CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList()),
              CVector3f::Zero());
          x420_curDamageRemTime = x424_damageWaitTime;
        }
      }
    }
    break;
  }
  case kSM_Deleted:
    mgr.DeleteObjectRequest(x590_);
    x594_collisionMgr->Destroy(mgr);
    break;
  case kSM_Activate:
    if (HasActorLights())
      ActorLights()->SetNeedsRelight(true);
    break;
  case kSM_Decrement:
    if (x5b4_state != 0 && x5b4_state != 5 && x5b4_state != 6 && x5b4_state != 4) {
      SetWasHit(true);
      x428_damageCooldownTimer = x424_damageWaitTime;
    }
    break;
  case kSM_SuspendedMove:
    if (x594_collisionMgr.get())
      x594_collisionMgr->SetMovable(mgr, false);
    break;
  }
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
  const bool active = GetActive();
  if (oldActive != active && x594_collisionMgr.get())
    x594_collisionMgr->SetActive(mgr, active);
}

void CSpankWeed::Think(float dt, CStateManager& mgr) {
  if (!GetActive())
    return;
  HealthInfo(mgr)->SetHP(1000000.f);
  if (!x598_isHiding) {
    const CVector3f scale = GetModelData()->GetScale();
    const CVector3f eyeOrigin = GetLocatorTransform(rstl::string_l("Eye")).GetTranslation();
    CVector3f offset = CVector3f::ByElementMultiply(scale, eyeOrigin);
    offset = GetTransform().Rotate(offset);
    MoveCollisionPrimitive(offset);
    x594_collisionMgr->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
    SetTransformDirty(true);
  }
  CPatterned::Think(dt, mgr);
}

void CSpankWeed::KnockBack(const CVector3f& backVec, CStateManager& mgr, const CDamageInfo& info,
                           float magnitude, bool direct, const bool inDeferred) {
  if (x57c_canKnockBack) {
    CPatterned::KnockBack(backVec, mgr, info, magnitude, direct, inDeferred);
    x57c_canKnockBack = false;
  }
}

void CSpankWeed::Patrol(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x460_knockBackController.SetEnableFreeze(false);
    x450_bodyController->SetLocomotionType(pas::kLT_Relaxed);
    RemoveMaterial(kMT_Solid, kMT_Scannable, mgr);
    RemoveMaterial(kMT_Orbit, kMT_Target, mgr);
    x594_collisionMgr->SetActive(mgr, false);
    x598_isHiding = true;
    x5b4_state = 0;
    break;
  case kStateMsg_Deactivate:
    AddMaterial(kMT_Orbit, kMT_Target, kMT_Scannable, mgr);
    SetTranslation(x584_retreatOrigin);
    x594_collisionMgr->SetActive(mgr, true);
    x598_isHiding = false;
    x460_knockBackController.SetEnableFreeze(true);
    x5b8_previousState = 0;
    break;
  }
}

void CSpankWeed::FadeIn(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x5bc_animPhase = 0;
    x57c_canKnockBack = true;
    x5b4_state = 5;
    break;
  case kStateMsg_Update:
    switch (x5bc_animPhase) {
    case 0:
      if (x450_bodyController->GetCurrentStateId() == pas::kAS_Step)
        x5bc_animPhase = 2;
      else
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCStepCmd(pas::kSD_Forward, pas::kStep_Normal));
      break;
    case 2:
      if (x450_bodyController->GetCurrentStateId() != pas::kAS_Step)
        x5bc_animPhase = 3;
      break;
    }
    break;
  case kStateMsg_Deactivate:
    x5b8_previousState = 5;
    break;
  }
  SetWorldLightingDirty(true);
}

void CSpankWeed::FadeOut(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x5bc_animPhase = 0;
    x57c_canKnockBack = false;
    x5b4_state = 6;
    break;
  case kStateMsg_Update:
    switch (x5bc_animPhase) {
    case 0:
      if (x450_bodyController->GetCurrentStateId() == pas::kAS_Step)
        x5bc_animPhase = 2;
      else
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCStepCmd(pas::kSD_Backward, pas::kStep_Normal));
      break;
    case 2:
      if (x450_bodyController->GetCurrentStateId() != pas::kAS_Step)
        x5bc_animPhase = 3;
      break;
    }
    break;
  case kStateMsg_Deactivate:
    x5b8_previousState = 6;
    break;
  }
}

void CSpankWeed::Lurk(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x460_knockBackController.SetEnableFreeze(true);
    x450_bodyController->SetLocomotionType(pas::kLT_Lurk);
    RemoveMaterial(kMT_Solid, mgr);
    x5b4_state = 1;
    break;
  case kStateMsg_Deactivate:
    x5b8_previousState = 1;
    break;
  }
}

void CSpankWeed::TargetPatrol(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x450_bodyController->SetLocomotionType(pas::kLT_Combat);
    RemoveMaterial(kMT_Solid, mgr);
    x5b4_state = 2;
    break;
  case kStateMsg_Deactivate:
    x5b8_previousState = 2;
    break;
  }
}

void CSpankWeed::Attack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x450_bodyController->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::kS_Zero));
    x5b4_state = 3;
    break;
  case kStateMsg_Update:
    if (x450_bodyController->GetCurrentStateId() != pas::kAS_MeleeAttack)
      x450_bodyController->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::kS_Zero));
    break;
  case kStateMsg_Deactivate:
    x5b8_previousState = 3;
    break;
  }
}

bool CSpankWeed::InDetectionRange(CStateManager& mgr, float arg) {
  float playerDist = GetPlayerDistance(mgr);
  if (x56c_detectionHeightRange > 0.f) {
    const CVector3f playerPos = mgr.GetPlayer()->GetTranslation();
    return CMath::AbsF(playerPos.GetZ() - GetTranslation().GetZ()) < x56c_detectionHeightRange &&
           playerDist < x568_maxDetectionRange * x568_maxDetectionRange;
  }
  return playerDist < x568_maxDetectionRange * x568_maxDetectionRange;
}

bool CSpankWeed::HearPlayer(CStateManager& mgr, float arg) {
  float playerDist = GetPlayerDistance(mgr);
  if (x56c_detectionHeightRange > 0.f) {
    const CVector3f playerPos = mgr.GetPlayer()->GetTranslation();
    return CMath::AbsF(playerPos.GetZ() - GetTranslation().GetZ()) < x56c_detectionHeightRange &&
           playerDist < x570_maxHearingRange * x570_maxHearingRange;
  }
  return playerDist < x570_maxHearingRange * x570_maxHearingRange;
}

bool CSpankWeed::InRange(CStateManager& mgr, float arg) {
  float playerDist = GetPlayerDistance(mgr);
  if (x56c_detectionHeightRange > 0.f) {
    const CVector3f playerPos = mgr.GetPlayer()->GetTranslation();
    return CMath::AbsF(playerPos.GetZ() - GetTranslation().GetZ()) < x56c_detectionHeightRange &&
           playerDist < x574_maxSightRange * x574_maxSightRange;
  }
  return playerDist < x574_maxSightRange * x574_maxSightRange;
}

bool CSpankWeed::Delay(CStateManager& mgr, float arg) {
  if (GetWasHit()) {
    if (x330_stateMachineState.GetTime() > x578_hideTime) {
      SetWasHit(false);
      return true;
    }
    return false;
  }
  return true;
}

void CSpankWeed::Flinch(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x5bc_animPhase = 0;
    x5b4_state = 4;
    RemoveMaterial(kMT_Orbit, kMT_Target, mgr);
    break;
  case kStateMsg_Update:
    switch (x5bc_animPhase) {
    case 0:
      if (x450_bodyController->GetCurrentStateId() == pas::kAS_KnockBack)
        x5bc_animPhase = 2;
      else
        x450_bodyController->CommandMgr().DeliverCmd(
            CBCKnockBackCmd(CVector3f::Zero(), pas::kS_Zero));
      break;
    case 2:
      if (x450_bodyController->GetCurrentStateId() != pas::kAS_KnockBack)
        x5bc_animPhase = 3;
      break;
    }
    break;
  case kStateMsg_Deactivate:
    x5b8_previousState = 4;
    break;
  }
}

CVector3f CSpankWeed::GetAimPosition(const CStateManager& mgr, float dt) const {
  CVector3f pos = CVector3f::Zero();
  if (dt > 0.f)
    pos = PredictMotion(dt).GetTranslation();
  const CAnimData& animData = *GetModelData()->GetAnimationData();
  const CSegId id = animData.GetLocatorSegId(rstl::string_l("lockon_target_LCTR"));
  if (id.val() != 0xff) {
    const CTransform4f locatorXf = animData.GetLocatorTransform(id, nullptr);
    const CVector3f scaledOrigin =
        CVector3f::ByElementMultiply(GetModelData()->GetScale(), locatorXf.GetTranslation());
    pos += GetTransform() * scaledOrigin;
  } else {
    pos += GetBoundingBox().GetCenterPoint();
  }
  return pos;
}

CVector3f CSpankWeed::GetOrbitPosition(const CStateManager& mgr) const {
  CVector3f ret = CPatterned::GetOrbitPosition(mgr);
  const float maxTime = 1.f;
  float time = rstl::min_val(x330_stateMachineState.GetTime(), maxTime);
  CVector3f target = GetTranslation() + x59c_lockonOffset;
  if (x5b4_state == 3 && x5b8_previousState == 2)
    return CVector3f::Lerp(ret, target, time);
  else if (x5b4_state == 2 && x5b8_previousState == 3)
    return CVector3f::Lerp(target, ret, time);
  return ret;
}

float CSpankWeed::GetPlayerDistance(CStateManager& mgr) const {
  return (mgr.GetPlayer()->GetTranslation() - x5a8_lockonTarget).MagSquared();
}

bool CSpankWeed::AnimOver(CStateManager& mgr, float arg) { return x5bc_animPhase == 3; }

bool CSpankWeed::Attacked(CStateManager& mgr, float arg) { return CPatterned::Attacked(mgr, arg); }

void CSpankWeed::Render(const CStateManager& mgr) const { CPatterned::Render(mgr); }

CSpankWeed::~CSpankWeed() {}
