#include "Runtime/MP1/World/CThardus.hpp"

#include <array>

#include "Runtime/Camera/CCameraManager.hpp"
#include "Runtime/Camera/CFirstPersonCamera.hpp"
#include "Runtime/Collision/CCollisionActor.hpp"
#include "Runtime/Collision/CCollisionActorManager.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/MP1/CSamusHud.hpp"
#include "Runtime/MP1/World/CThardusRockProjectile.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CDestroyableRock.hpp"
#include "Runtime/World/CGameLight.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CScriptDistanceFog.hpp"
#include "Runtime/World/CScriptWaypoint.hpp"
#include "Runtime/World/CWorld.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

#include <DataSpec/DNAMP1/SFX/IceWorld.h>
namespace urde::MP1 {
namespace {
constexpr std::array<SSphereJointInfo, 7> skDamageableSphereJointInfoList1{{
    {"R_Knee", 1.f},
    {"R_Elbow_Collision_LCTR", 1.5f},
    {"L_Elbow_Collision_LCTR", 1.5f},
    {"L_Knee_Collision_LCTR", 1.f},
    {"R_Back_Rock_Collision_LCTR", 2.5f},
    {"L_Back_Rock_Collision_LCTR", 1.5f},
    {"Head_Collision_LCTR", 1.5f},
}};

constexpr std::array<SSphereJointInfo, 5> skDamageableSphereJointInfoList2{{
    {"R_Shoulder_Collision_LCTR", 0.75f},
    {"L_Shoulder_Collision_LCTR", 0.75f},
    {"Spine_Collision_LCTR", 0.75f},
    {"R_Hand_Collision_LCTR", 2.25f},
    {"L_Hand_Collision_LCTR", 2.f},
}};

constexpr std::array<SAABoxJointInfo, 2> skFootCollision{{
    {"R_Foot_Collision_LCTR", zeus::CVector3f(3.f, 3.f, 1.f)},
    {"L_Foot_Collision_LCTR", zeus::CVector3f(3.f, 2.f, 3.f)},
}};

constexpr std::array skSearchJointNames{
    "R_Knee"sv,
    "R_Elbow_Collision_LCTR"sv,
    "L_Elbow_Collision_LCTR"sv,
    "L_Knee_Collision_LCTR"sv,
    "R_Back_Rock_Collision_LCTR"sv,
    "L_Back_Rock_Collision_LCTR"sv,
    "Head_Collision_LCTR"sv,
};

constexpr std::array skRockJoints{
    "R_knee"sv, "R_forearm"sv, "L_elbow"sv, "L_hip"sv, "R_collar_BigRock_SDK"sv, "collar_rock4_SDK"sv, "Neck_1"sv,
};
} // Anonymous namespace

CThardus::CThardus(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                   CModelData&& mData, const CActorParameters& actParms, const CPatternedInfo& pInfo,
                   std::vector<CStaticRes> mData1, std::vector<CStaticRes> mData2, CAssetId particle1,
                   CAssetId particle2, CAssetId particle3, float f1, float f2, float f3, float f4, float f5, float f6,
                   CAssetId stateMachine, CAssetId particle4, CAssetId particle5, CAssetId particle6,
                   CAssetId particle7, CAssetId particle8, CAssetId particle9, CAssetId texture, u32 sfxId1,
                   CAssetId particle10, u32 sfxId2, u32 sfxId3, u32 sfxId4)
: CPatterned(ECharacter::Thardus, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Ground, EColliderType::One, EBodyType::BiPedal, actParms, EKnockBackVariant::Large)
, x5cc_(std::move(mData1))
, x5dc_(std::move(mData2))
, x600_(particle1)
, x604_(particle2)
, x608_(particle3)
, x630_(stateMachine)
, x694_(f1)
, x698_(f2)
, x69c_(f3)
, x6a0_(f4)
, x6a4_(f5)
, x6a8_(f6)
, x6d0_(particle4)
, x6d4_(particle5)
, x6d8_(particle6)
, x6dc_(particle7)
, x6e0_(particle8)
, x6e4_(particle9)
, x6e8_(texture)
, x6ec_(CSfxManager::TranslateSFXID(sfxId1))
, x6f0_(particle10)
, x758_(sfxId2)
, x75c_(sfxId3)
, x760_(sfxId4)
, x7f0_pathFindSearch(nullptr, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f) {
  CMaterialList exclude = GetMaterialFilter().GetExcludeList();
  CMaterialList include = GetMaterialFilter().GetIncludeList();
  exclude.Add(EMaterialTypes::Player);
  exclude.Add(EMaterialTypes::Character);
  exclude.Add(EMaterialTypes::CollisionActor);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));
  std::vector<CAssetId> gens;
  gens.reserve(6);
  gens.push_back(particle4);
  gens.push_back(particle5);
  gens.push_back(particle6);
  gens.push_back(particle7);
  gens.push_back(particle8);
  gens.push_back(particle9);
  GetModelData()->GetAnimationData()->GetParticleDB().CacheParticleDesc(
      CCharacterInfo::CParticleResData(std::move(gens), {}, {}, {}));
  x798_.reserve(6);
  x7a8_timers.reserve(16);
  UpdateThermalFrozenState(true);
  xd0_damageMag = 0.f;
  x50c_baseDamageMag = 0.f;
  x8f4_waypoints.reserve(16);
  x91c_flareTexture = g_SimplePool->GetObj("Thermal_Spot_2"sv);
  x91c_flareTexture.Lock();
  x403_26_stateControlledMassiveDeath = false;
  x460_knockBackController.SetAutoResetImpulse(false);
  SetMass(100000.f);
}

void CThardus::sub801dbf34(float dt, CStateManager& mgr) {}
void CThardus::sub801dd608(CStateManager& mgr) {
  zeus::CVector3f scale = GetModelData()->GetScale();
  CAnimData* animData = GetModelData()->GetAnimationData();
  for (size_t i = 0; i < x610_destroyableRocks.size(); ++i) {
    zeus::CTransform xf =
        GetTransform() * (zeus::CTransform::Scale(scale) * animData->GetLocatorTransform(skRockJoints[i], nullptr));
    if (TCastToPtr<CActor> act = mgr.ObjectById(x610_destroyableRocks[i]))
      act->SetTransform(xf);
    if (TCastToPtr<CGameLight> gl = mgr.ObjectById(x6c0_rockLights[i]))
      gl->SetTransform(xf);
  }
}
void CThardus::sub801dcfa4(CStateManager& mgr) {}

void CThardus::Think(float dt, CStateManager& mgr) {
  if (!GetActive() && x450_bodyController->GetActive()) {
    x91c_flareTexture.Lock();
  }

  if (x7c8_) {
    float fVar2 = 10.f * GetModelData()->GetScale().x();
    zeus::CVector3f diff = mgr.GetPlayer().GetTranslation() - GetTranslation();
    if (diff.magSquared() < fVar2 * fVar2) {
      mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer().GetUniqueId(), GetUniqueId(),
                      CDamageInfo(CWeaponMode(EWeaponType::AI), 0.f, 0.f, 10.f), CMaterialFilter::skPassEverything,
                      diff.normalized());
      x688_ = true;
      x7c8_ = false;
      x7cc_.zeroOut();
    }
  }

  sub801dbf34(dt, mgr);

  if (!sub801dc2c8()) {
    if (x648_currentRock < x610_destroyableRocks.size() - 2)
      x690_ = 1.f;
    else
      x690_ = 1.f;
  } else {
    x690_ = 1.f;
    SendScriptMsgs(EScriptObjectState::DeactivateState, mgr, EScriptObjectMessage::None);
  }

  if (!x93c_) {
    x3b4_speed = x690_;
    x402_28_isMakingBigStrike = false;
    x504_damageDur = 0.f;
  } else {
    x3b4_speed = x690_;
    x402_28_isMakingBigStrike = true;
    x504_damageDur = 1.f;
  }

  CPatterned::Think(dt, mgr);

  if (x648_currentRock > 2 && !x689_) {
    _DoSuckState(mgr);
  }

  x5f0_rockColliders->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);
  x5f4_->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);
  x5f8_->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);

  sub801dd608(mgr);
  sub801dcfa4(mgr);

  if (x610_destroyableRocks.size() <= x648_currentRock)
    Death(mgr, zeus::skZero3f, EScriptObjectState::DeathRattle);

  if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::Thermal) {
    for (size_t i = 0; i < x610_destroyableRocks.size(); ++i) {
      if (CActor* act = static_cast<CActor*>(mgr.ObjectById(x610_destroyableRocks[i]))) {
        if (x648_currentRock == i && !x688_ && !x93c_ && !x909_ && !x93d_) {
          act->AddMaterial(EMaterialTypes::Orbit, mgr);
          act->AddMaterial(EMaterialTypes::Target, mgr);
        } else {
          RemoveMaterial(EMaterialTypes::Orbit, mgr);
          RemoveMaterial(EMaterialTypes::Target, mgr);
        }
      }
    }
    if (x688_)
      x688_ = false;

  } else {
    UpdateNonDestroyableCollisionActorMaterials(EUpdateMaterialMode::Remove, EMaterialTypes::ProjectilePassthrough,
                                                mgr);
    for (size_t i = 0; i < x610_destroyableRocks.size(); ++i) {
      if (CActor* act = static_cast<CActor*>(mgr.ObjectById(x610_destroyableRocks[i]))) {
        if (!x688_ && !x93c_ && !x909_ && !x93d_) {
          bool found = act->GetName().find("Neck_1"sv) != std::string::npos;
          if (!found || !x6b0_[x648_currentRock] || x648_currentRock == x610_destroyableRocks.size() - 1) {
            if (!x6b0_[i]) {
              if (!found || x6b0_[i]) {
                act->RemoveMaterial(EMaterialTypes::Orbit, mgr);
                act->RemoveMaterial(EMaterialTypes::Target, mgr);
              }
            } else {
              act->AddMaterial(EMaterialTypes::Orbit, mgr);
              act->AddMaterial(EMaterialTypes::Target, mgr);
            }
          } else {
            act->RemoveMaterial(EMaterialTypes::Orbit, mgr);
            act->RemoveMaterial(EMaterialTypes::Target, mgr);
          }
        } else {
          x688_ = false;
          act->RemoveMaterial(EMaterialTypes::Orbit, mgr);
          act->RemoveMaterial(EMaterialTypes::Target, mgr);
        }
      }
    }
  }
  if (x644_ == 1) {
    UpdateExcludeList(x5f0_rockColliders, EUpdateMaterialMode::Add, EMaterialTypes::Player, mgr);
    UpdateExcludeList(x5f4_, EUpdateMaterialMode::Add, EMaterialTypes::Player, mgr);
    UpdateExcludeList(x5f8_, EUpdateMaterialMode::Add, EMaterialTypes::Player, mgr);
    CCameraManager* cameraManager = mgr.GetCameraManager();
    if (x93c_) {
      if (x6f8_ < 0.3f) {
        x6f8_ += dt;
      } else {
        if (cameraManager->GetFirstPersonCamera()->GetUniqueId() == cameraManager->GetCurrentCameraId()) {
          cameraManager->AddCameraShaker(
              CCameraShakeData::BuildMissileCameraShake(0.25f, 0.75f, 125.f, GetTranslation()), true);
        }
        x6f8_ = 0.f;
      }

      if (cameraManager->GetCurrentCameraId() != cameraManager->GetFirstPersonCamera()->GetUniqueId() && x95d_ == 0) {
        CSamusHud::DisplayHudMemo(g_MainStringTable->GetString(104), CHUDMemoParms(5.f, true, false, false));
      }
    }
  } else {
    UpdateExcludeList(x5f0_rockColliders, EUpdateMaterialMode::Remove, EMaterialTypes::Player, mgr);
    UpdateExcludeList(x5f4_, EUpdateMaterialMode::Remove, EMaterialTypes::Player, mgr);
    UpdateExcludeList(x5f8_, EUpdateMaterialMode::Remove, EMaterialTypes::Player, mgr);
  }
}
void CThardus::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);

  switch (msg) {
  case EScriptObjectMessage::Reset: {
    x95c_doCodeTrigger = true;
    x450_bodyController->SetFallState(pas::EFallState::Zero);
    x450_bodyController->SetState(pas::EAnimationState::Locomotion);
    x93d_ = false;
    break;
  }
  case EScriptObjectMessage::SetToMax: {
    for (size_t i = x648_currentRock; i < x610_destroyableRocks.size() - 1; ++i) {
      if (CEntity* ent = mgr.ObjectById(x610_destroyableRocks[i])) {
        ent->SetActive(false);
      }
      ++x648_currentRock;
    }
    break;
  }
  case EScriptObjectMessage::Stop: {
    Death(mgr, {}, EScriptObjectState::DeathRattle);
    break;
  }
  case EScriptObjectMessage::Action: {
    if (!x5c8_heardPlayer)
      x5c8_heardPlayer = true;
    break;
  }
  case EScriptObjectMessage::Touched: {
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(uid)) {
      if (TCastToPtr<CPlayer> pl = mgr.ObjectById(colAct->GetLastTouchedObject())) {
        if (x420_curDamageRemTime > 0.f)
          break;
        u32 rand = mgr.GetActiveRandom()->Next();
        float damageMult = 1.f;
        zeus::CVector3f knockBack = zeus::skForward;
        if (x644_ == 1) {
          damageMult = 2.f;
          knockBack = (rand % 2) ? zeus::skRight : zeus::skLeft;
        }

        if (mgr.GetPlayer().GetFrozenState())
          mgr.GetPlayer().UnFreeze(mgr);

        knockBack = GetTransform().buildMatrix3f() * knockBack;
        CDamageInfo dInfo = GetContactDamage();
        dInfo.SetDamage(damageMult * dInfo.GetDamage());
        mgr.ApplyDamage(GetUniqueId(), pl->GetUniqueId(), GetUniqueId(), dInfo,
                        CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}),
                        x644_ == 1 ? knockBack : zeus::skZero3f);
        x420_curDamageRemTime = x424_damageWaitTime;
      } else if (TCastToConstPtr<CBomb>(mgr.GetObjectById(colAct->GetLastTouchedObject()))) {
        if (x644_ == 1 && x93c_)
          sub801dae2c(mgr, x648_currentRock);
      }
    }

    break;
  }
  case EScriptObjectMessage::Registered: {
    x610_destroyableRocks.reserve(x5cc_.size());
    x6b0_.reserve(x5cc_.size());
    x6c0_rockLights.reserve(x5cc_.size());
    x90c_.reserve(x5cc_.size());
    for (size_t i = 0; i < x5cc_.size(); ++i) {
      float health = (i == x5cc_.size() - 1) ? 2.f * x6a8_ : x6a8_;
      TUniqueId rockId = mgr.AllocateUniqueId();
      CModelData mData1(x5cc_[i]);
      mgr.AddObject(new CDestroyableRock(
          rockId, true, "", CEntityInfo(GetAreaIdAlways(), NullConnectionList), {}, std::move(mData1), 0.f,
          CHealthInfo(health, 0.f),
          CDamageVulnerability(
              EVulnerability::Normal, EVulnerability::Deflect, EVulnerability::Normal, EVulnerability::Normal,
              EVulnerability::Normal, EVulnerability::Normal, EVulnerability::Normal, EVulnerability::Deflect,
              EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect,
              EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect, EDeflectType::One),
          GetMaterialList(), x630_,
          CActorParameters(CLightParameters(false, 0.f, CLightParameters::EShadowTesselation::Invalid, 0.f, 0.f,
                                            zeus::skWhite, true, CLightParameters::EWorldLightingOptions::NoShadowCast,
                                            CLightParameters::ELightRecalculationOptions::LargeFrameCount,
                                            zeus::skZero3f, -1, -1, false, 0),
                           {}, {}, {}, {}, true, true, false, false, 0.f, 0.f, 1.f),
          x5dc_[i], 0));
      x610_destroyableRocks.push_back(rockId);
      x6b0_.push_back(false);
      TUniqueId lightId = mgr.AllocateUniqueId();
      CGameLight* gl = new CGameLight(lightId, GetAreaIdAlways(), false, ""sv, {}, GetUniqueId(),
                                      CLight::BuildPoint({}, zeus::skBlue), 0, 0, 0.f);
      gl->SetActive(false);
      mgr.AddObject(gl);
      x6c0_rockLights.push_back(lightId);
      x90c_.push_back(health);
    }

    AddMaterial(EMaterialTypes::ScanPassthrough, mgr);
    AddMaterial(EMaterialTypes::CameraPassthrough, mgr);
    RemoveMaterial(EMaterialTypes::Orbit, mgr);
    RemoveMaterial(EMaterialTypes::Target, mgr);
    _SetupCollisionManagers(mgr);
    x450_bodyController->SetFallState(pas::EFallState::Two);
    x450_bodyController->Activate(mgr);
    x450_bodyController->BodyStateInfo().SetLocoAnimChangeAtEndOfAnimOnly(true);
    SetState(0, mgr);
    sub801dec80();
    AddMaterial(EMaterialTypes::RadarObject, mgr);
    break;
  }
  case EScriptObjectMessage::Deleted: {
    x5f0_rockColliders->Destroy(mgr);
    x5f4_->Destroy(mgr);
    x5f8_->Destroy(mgr);
    mgr.FreeScriptObject(x64c_fog);
    for (const auto& id : x610_destroyableRocks) {
      mgr.FreeScriptObject(id);
    }

    for (const auto& id : x6c0_rockLights) {
      mgr.FreeScriptObject(id);
    }
    break;
  }
  case EScriptObjectMessage::InitializedInArea: {
    if (x94c_initialized)
      break;
    x94c_initialized = true;
    x764_startTransform = GetTransform();

    for (const SConnection& conn : GetConnectionList()) {
      TUniqueId connId = mgr.GetIdForScript(conn.x8_objId);
      if (connId == kInvalidUniqueId)
        continue;

      if (conn.x0_state == EScriptObjectState::Patrol) {
        if (TCastToPtr<CScriptWaypoint> wp = mgr.ObjectById(connId)) {
          rstl::reserved_vector<TUniqueId, 16> wpIds;
          GatherWaypoints(wp, mgr, wpIds);
          for (const auto& id : wpIds)
            x574_waypoints.push_back(id);
        } else if (CPatterned* p = CPatterned::CastTo<CThardusRockProjectile>(mgr.ObjectById(connId))) {
          x5fc_projectileId = connId;
          x60c_projectileEditorId = conn.x8_objId;
          p->SetActive(false);
        } else if (TCastToConstPtr<CRepulsor>(mgr.GetObjectById(connId))) {
          x660_repulsors.push_back(connId);
        } else if (TCastToConstPtr<CScriptDistanceFog>(mgr.GetObjectById(connId))) {
          x64c_fog = connId;
        }
      } else if (conn.x0_state == EScriptObjectState::Zero) {
        if (TCastToPtr<CScriptWaypoint> wp = mgr.ObjectById(connId)) {
          x8f4_waypoints.push_back(connId);
          wp->SetActive(false);
        }
      } else if (conn.x0_state == EScriptObjectState::Dead) {
        if (TCastToConstPtr<CScriptTimer>(mgr.GetObjectById(connId)))
          x7a8_timers.push_back(connId);
      }
    }
    x7f0_pathFindSearch.SetArea(mgr.GetWorld()->GetAreaAlways(GetAreaIdAlways())->GetPostConstructed()->x10bc_pathArea);
    break;
  }
  case EScriptObjectMessage::Damage: {
    break;
  }
  default:
    break;
  }
}

void CThardus::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) {
  CPatterned::PreRender(mgr, frustum);
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::Thermal) {
    xb4_drawFlags = CModelFlags(0, 0, 1, zeus::skWhite);
  } else {
    xb4_drawFlags = CModelFlags(0, 0, 3, zeus::skWhite);
  }
}
void CThardus::Render(CStateManager& mgr) {
  CPatterned::Render(mgr);
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::Thermal && x7c4_ != 0) {
    RenderFlare(mgr, x7c0_);
  }
}
void CThardus::Touch(CActor& act, CStateManager& mgr) { CPatterned::Touch(act, mgr); }
zeus::CVector3f CThardus::GetOrbitPosition(const CStateManager& mgr) const { return GetAimPosition(mgr, 0.f); }
zeus::CVector3f CThardus::GetAimPosition(const CStateManager& mgr, float dt) const {
  return GetLctrTransform(x93c_ ? "center_LCTR"sv : "Neck_1"sv).origin;
}
zeus::CAABox CThardus::GetSortingBounds(const CStateManager& mgr) const {
  zeus::CVector3f extents = 0.15f * (x9c_renderBounds.max - x9c_renderBounds.min);
  return zeus::CAABox(x9c_renderBounds.min - extents, x9c_renderBounds.max + extents);
}

void CThardus::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}
void CThardus::Patrol(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x658_ = -1;
    x950_ = GetTranslation();
  } else if (msg == EStateMsg::Update) {
    if (!ShouldTurn(mgr, 0.f))
      return;

    zeus::CVector3f plPos = mgr.GetPlayer().GetTranslation();
    zeus::CQuaternion q = zeus::CQuaternion::lookAt(x950_, plPos.normalized(), zeus::degToRad(360.f));
    x450_bodyController->GetCommandMgr().DeliverCmd(
        CBCLocomotionCmd(zeus::skZero3f, q.toTransform() * (plPos - GetTranslation()).normalized(), 1.f));
    x950_ = plPos;
  } else if (msg == EStateMsg::Deactivate) {
    sub80deadc(mgr);
    x94d_ = false;
  }
}
void CThardus::Dead(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    SetTransform(x764_startTransform);
  }
  CPatterned::Dead(mgr, msg, arg);
}
void CThardus::PathFind(CStateManager& mgr, EStateMsg msg, float arg) {
  CPatterned::PathFind(mgr, msg, arg);
  return;
  if (msg == EStateMsg::Activate) {
    x2e0_destPos = sub801de550(mgr);
    x7e4_ = x7d8_ = x2e0_destPos;
    CPatterned::PathFind(mgr, EStateMsg::Activate, arg);
  } else if (msg == EStateMsg::Update) {

  } else if (msg == EStateMsg::Deactivate) {
    x8d4_ = false;
  }
}
void CThardus::TargetPatrol(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x5ec_ = 0;
    if (x95e_)
      return;

    mgr.SetBossParams(GetUniqueId(), GetHealthInfo(mgr)->GetHP(), 88);
    x95e_ = true;
  } else if (msg == EStateMsg::Update) {
    if (x5ec_ == 0) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Taunt)
        x5ec_ = 2;
      else
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCTauntCmd(pas::ETauntType::One));
    } else if (x5ec_ == 2 &&
               x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Taunt) {
      x5ec_ = 3;
    }
  }
}
void CThardus::Generate(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x5ec_ = 0;
  } else if (msg == EStateMsg::Update) {
    if (x5ec_ == 0) {
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Getup) {
        x5ec_ = 2;
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCGetupCmd(pas::EGetupType::Zero));
      }
    } else if (x5ec_ == 2 && x450_bodyController->GetCurrentStateId() != pas::EAnimationState::Getup) {
      x5ec_ = 3;
    }
  } else if (msg == EStateMsg::Deactivate) {
    x93d_ = false;
  }
}
void CThardus::Attack(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x5ec_ = 0;
    ++x570_;
    x5ec_ = 0;
  } else if (msg == EStateMsg::Update) {
    if (x5ec_ == 0) {
      if (GetBodyController()->GetCurrentStateId() != pas::EAnimationState::MeleeAttack) {
        if (mgr.GetActiveRandom()->Float() <= 0.5f) {
          GetBodyController()->GetCommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::ESeverity::One));
        } else {
          GetBodyController()->GetCommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::ESeverity::Zero));
        }
        ++x570_;
      } else {
        x5ec_ = 2;
      }
    } else if (x5ec_ == 2 && GetBodyController()->GetCurrentStateId() != pas::EAnimationState::MeleeAttack) {
      x5ec_ = 3;
    }
  }
}
void CThardus::LoopedAttack(CStateManager& mgr, EStateMsg msg, float arg) { CAi::LoopedAttack(mgr, msg, arg); }
void CThardus::DoubleSnap(CStateManager& mgr, EStateMsg msg, float arg) { }
void CThardus::Shuffle(CStateManager& mgr, EStateMsg msg, float arg) {  }
void CThardus::GetUp(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg != EStateMsg::Activate)
    return;
  RemoveMaterial(EMaterialTypes::RadarObject, mgr);
}

void CThardus::Taunt(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x5ec_ = 0;
  } else if (msg == EStateMsg::Update) {
    if (x5ec_ == 0) {
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Taunt)
        x5ec_ = 2;
      else
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCTauntCmd(pas::ETauntType::One));
    } else if (x5ec_ == 2 && x450_bodyController->GetCurrentStateId() != pas::EAnimationState::Taunt) {
      x5ec_ = 3;
    }
  }
}

void CThardus::Suck(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x909_ = true;
    x93d_ = true;
    SendScriptMsgs(EScriptObjectState::MaxReached, mgr, EScriptObjectMessage::None);
  } else if (msg == EStateMsg::Deactivate) {
    x689_ = true;
  }
}
void CThardus::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x5ec_ = 0;
  } else if (msg == EStateMsg::Update) {
    if (x5ec_ == 1)
      return;

    if (x5ec_ == 0) {
      if (GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::ProjectileAttack) {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCProjectileAttackCmd(pas::ESeverity::Zero, {}, false));
        x5ec_ = 0;
      } else {
        x5ec_ = 2;
      }
    } else if (x5ec_ == 2 &&
               GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::ProjectileAttack) {
      x5ec_ = 3;
    }
  }
}
void CThardus::Flinch(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    for (TUniqueId uid : x798_) {
      if (CThardusRockProjectile* rock = CPatterned::CastTo<CThardusRockProjectile>(mgr.ObjectById(uid)))
        rock->sub80203d58();
    }
    x93b_ = true;
    x93d_ = true;
    x909_ = true;
    x93c_ = false;
    SetState(-1, mgr);
    x94d_ = true;
  } else if (msg == EStateMsg::Update) {
    pas::ESeverity severity = pas::ESeverity::Invalid;
    switch (x648_currentRock) {
    case 1:
      severity = pas::ESeverity::Zero;
      break;
    case 2:
      severity = pas::ESeverity::One;
      break;
    case 3:
      severity = pas::ESeverity::Two;
      break;
    case 4:
      severity = pas::ESeverity::Three;
      break;
    case 5:
      severity = pas::ESeverity::Four;
      break;
    case 6:
      severity = pas::ESeverity::Five;
      break;
    }

    if (x5ec_ == 0) {
      if (GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::KnockBack)
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCKnockBackCmd({}, severity));
      else
        x5ec_ = 2;
    } else if (x5ec_ == 2 &&
               GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::KnockBack) {
      x5ec_ = 3;
    }
  }
}
void CThardus::TelegraphAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x5ec_ = 0;
  } else if (msg == EStateMsg::Update) {
    if (x5ec_ == 0) {
      if (GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::ProjectileAttack) {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCProjectileAttackCmd(pas::ESeverity::One, {}, false));
        x5ec_ = 0;
      } else
        x5ec_ = 2;
    } else if (x5ec_ == 2 &&
               GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::ProjectileAttack) {
      x5ec_ = 3;
    }
  }
}
void CThardus::Explode(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x5ec_ = 0;
    CSfxManager::SfxStop(x904_);
    x909_ = true;
    x93d_ = true;
    x909_ = true;
    SendScriptMsgs(EScriptObjectState::Arrived, mgr, EScriptObjectMessage::None);
  } else if (msg == EStateMsg::Update) {
    if (x5ec_ == 0) {
      if (GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Step) {
        GetBodyController()->GetCommandMgr().DeliverCmd(
            CBCStepCmd(pas::EStepDirection::Forward, pas::EStepType::Dodge));
      } else {
        x5ec_ = 2;
      }
    } else if (x5ec_ == 2 &&
               GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Step) {
      x5ec_ = 3;
    }
  } else if (msg == EStateMsg::Deactivate) {
    x8f0_ = true;
    x909_ = false;
    x93d_ = false;
  }
}
void CThardus::Cover(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    SetState(1, mgr);
    x93d_ = false;
    x909_ = false;
    if (x610_destroyableRocks.size() - 2 <= x648_currentRock)
      x690_ = 1.1f;
    AddMaterial(EMaterialTypes::RadarObject, mgr);
  } else if (msg == EStateMsg::Deactivate) {
    x690_ = 1.f;
  }
}
void CThardus::Enraged(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x5ec_ = 0;
    x688_ = true;
    x908_ = true;
  } else if (msg == EStateMsg::Update) {
    if (x5ec_ == 0) {
      if (GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Taunt) {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCTauntCmd(pas::ETauntType::Zero));
      } else {
        x5ec_ = 2;
      }
    } else if (x5ec_ == 2 &&
               GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Taunt) {
      x5ec_ = 3;
    }
  } else if (msg == EStateMsg::Deactivate) {
    x908_ = false;
  }
}
void CThardus::Growth(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x5ec_ = 0;
    x904_ = CSfxManager::SfxStart(SFXsfx07AD, 1.f, 0.f, false, 0x7f, true, GetAreaIdAlways());
  } else if (msg == EStateMsg::Update) {
    if (x5ec_ == 0) {
      if (GetBodyController()->GetCurrentStateId() != pas::EAnimationState::Step) {
        GetBodyController()->GetCommandMgr().DeliverCmd(
            CBCStepCmd(pas::EStepDirection::Forward, pas::EStepType::BreakDodge));
      } else {
        x5ec_ = 2;
      }
    } else if (x5ec_ == 2 && GetBodyController()->GetCurrentStateId() != pas::EAnimationState::Step) {
      x5ec_ = 3;
    }
  } else if (msg == EStateMsg::Deactivate) {
    if (TCastToPtr<CScriptDistanceFog> fog = mgr.ObjectById(x64c_fog)) {
      mgr.SendScriptMsg(GetUniqueId(), mgr.GetEditorIdForUniqueId(fog->GetUniqueId()), EScriptObjectMessage::Activate,
                        EScriptObjectState::Any);
      SendScriptMsgs(EScriptObjectState::Play, mgr, EScriptObjectMessage::None);
    }
  }
}
void CThardus::Faint(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x5ec_ = 0;
    x93c_ = false;
    SetState(-1, mgr);
    for (TUniqueId uid : x798_) {
      if (CThardusRockProjectile* rock = CPatterned::CastTo<CThardusRockProjectile>(mgr.ObjectById(uid)))
        rock->sub80203d58();
    }
    x94d_ = true;
  } else if (msg == EStateMsg::Update) {
    if (x5ec_ == 0) {
      if (GetBodyController()->GetCurrentStateId() != pas::EAnimationState::KnockBack) {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCKnockBackCmd({}, pas::ESeverity::Six));
      } else {
        x5ec_ = 2;
      }
    } else if (x5ec_ == 2 && GetBodyController()->GetCurrentStateId() != pas::EAnimationState::KnockBack) {
      x5ec_ = 3;
    }
  }
}

bool CThardus::PathFound(CStateManager& mgr, float arg) { return x8d4_; }
bool CThardus::InRange(CStateManager& mgr, float arg) {
  return (mgr.GetPlayer().GetTranslation().toVec2f() - GetTranslation().toVec2f()).magnitude() <
         10.f * GetModelData()->GetScale().x();
}
bool CThardus::PatternOver(CStateManager& mgr, float arg) { return !x574_waypoints.empty() || x93b_; }
bool CThardus::AnimOver(CStateManager& mgr, float arg) { return x5ec_ == 3; }
bool CThardus::InPosition(CStateManager& mgr, float arg) { return CPatterned::InPosition(mgr, arg); }
bool CThardus::ShouldTurn(CStateManager& mgr, float arg) { return CAi::ShouldTurn(mgr, arg); }
bool CThardus::HitSomething(CStateManager& mgr, float arg) { return CAi::HitSomething(mgr, arg); }

void CThardus::GatherWaypoints(urde::CScriptWaypoint* wp, urde::CStateManager& mgr,
                               rstl::reserved_vector<TUniqueId, 16>& uids) {
  if (uids.size() < uids.capacity() && wp->GetActive()) {
    uids.push_back(wp->GetUniqueId());
    wp->SetActive(false);
    for (const SConnection& conn : wp->GetConnectionList()) {
      TUniqueId uid = mgr.GetIdForScript(conn.x8_objId);
      if (TCastToPtr<CScriptWaypoint> wp2 = mgr.ObjectById(uid))
        GatherWaypoints(wp2, mgr, uids);
    }
    wp->SetActive(true);
  }
}

void CThardus::_BuildSphereJointList(const SSphereJointInfo* arr, size_t count,
                                     std::vector<CJointCollisionDescription>& list) {
  for (size_t i = 0; i < count; ++i) {
    const auto& jInfo = arr[i];
    list.push_back(CJointCollisionDescription::SphereCollision(
        GetModelData()->GetAnimationData()->GetLocatorSegId(jInfo.name), jInfo.radius, jInfo.name, 0.001f));
  }
}

void CThardus::_BuildAABoxJointList(const SAABoxJointInfo* arr, size_t count,
                                    std::vector<CJointCollisionDescription>& list) {
  for (size_t i = 0; i < count; ++i) {
    const auto& jInfo = arr[i];
    list.push_back(CJointCollisionDescription::AABoxCollision(
        GetModelData()->GetAnimationData()->GetLocatorSegId(jInfo.name), jInfo.extents, jInfo.name, 0.001f));
  }
}

void CThardus::_SetupCollisionActorMaterials(const std::unique_ptr<CCollisionActorManager>& colMgr,
                                             CStateManager& mgr) {
  for (size_t i = 0; i < colMgr->GetNumCollisionActors(); ++i) {
    const auto& colDesc = colMgr->GetCollisionDescFromIndex(i);
    if (CActor* act = static_cast<CActor*>(mgr.ObjectById(colDesc.GetCollisionActorId()))) {
      act->AddMaterial(EMaterialTypes::ScanPassthrough, mgr);
      act->AddMaterial(EMaterialTypes::CameraPassthrough, mgr);
      act->AddMaterial(EMaterialTypes::Immovable, mgr);
      act->AddMaterial(EMaterialTypes::NoPlayerCollision, mgr);
      CMaterialList include = GetMaterialFilter().GetIncludeList();
      include.Add(act->GetMaterialFilter().GetIncludeList());
      CMaterialList exclude = GetMaterialFilter().GetExcludeList();
      exclude.Add(act->GetMaterialFilter().GetExcludeList());
      act->SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));
    }
  }
}

void CThardus::_SetupCollisionManagers(CStateManager& mgr) {
  std::vector<CJointCollisionDescription> list;
  _BuildSphereJointList(skDamageableSphereJointInfoList1.data(), skDamageableSphereJointInfoList1.size(), list);
  x5f0_rockColliders = std::make_unique<CCollisionActorManager>(mgr, GetUniqueId(), GetAreaIdAlways(), list, true);
  _SetupCollisionActorMaterials(x5f0_rockColliders, mgr);
  list.clear();
  _BuildSphereJointList(skDamageableSphereJointInfoList2.data(), skDamageableSphereJointInfoList2.size(), list);
  x5f4_ = std::make_unique<CCollisionActorManager>(mgr, GetUniqueId(), GetAreaIdAlways(), list, true);
  _SetupCollisionActorMaterials(x5f4_, mgr);
  list.clear();
  _BuildAABoxJointList(skFootCollision.data(), skFootCollision.size(), list);
  x5f8_ = std::make_unique<CCollisionActorManager>(mgr, GetUniqueId(), GetAreaIdAlways(), list, true);
  _SetupCollisionActorMaterials(x5f8_, mgr);
  list.clear();
  x634_nonDestroyableActors.reserve(x5f4_->GetNumCollisionActors() + x5f0_rockColliders->GetNumCollisionActors() +
                                    x5f8_->GetNumCollisionActors());
  sub801dd4fc(x5f4_);
  sub801dd4fc(x5f8_);
  for (size_t i = 0; i < x5f0_rockColliders->GetNumCollisionActors(); ++i) {
    const auto& colDesc = x5f0_rockColliders->GetCollisionDescFromIndex(i);
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(colDesc.GetCollisionActorId())) {
      if (CDestroyableRock* rock = static_cast<CDestroyableRock*>(mgr.ObjectById(x610_destroyableRocks[i]))) {
        if (i == 0) {
          colAct->SetDamageVulnerability(*rock->GetDamageVulnerability());
          rock->Set_x32c(0.8f);
        } else {
          colAct->SetDamageVulnerability(CDamageVulnerability::ImmuneVulnerabilty());
          rock->Set_x32c(0.f);
        }

        *colAct->HealthInfo(mgr) = *rock->HealthInfo(mgr);
      }
    }
  }
}

void CThardus::sub801dd4fc(const std::unique_ptr<CCollisionActorManager>& colMgr) {
  for (size_t i = 0; i < colMgr->GetNumCollisionActors(); ++i) {
    const auto& colDesc = colMgr->GetCollisionDescFromIndex(i);
    TUniqueId uid = colDesc.GetCollisionActorId();
    bool foundBone = false;
    for (const auto& name : skSearchJointNames) {
      if (colDesc.GetName().find(name) != std::string::npos) {
        foundBone = true;
        break;
      }
    }

    if (!foundBone)
      x634_nonDestroyableActors.push_back(uid);
  }
}

void CThardus::sub801dae2c(CStateManager& mgr, u32 rockIndex) {
  TCastToPtr<CCollisionActor> rockCol =
      mgr.ObjectById(x5f0_rockColliders->GetCollisionDescFromIndex(rockIndex).GetCollisionActorId());
  if (TCastToPtr<CDestroyableRock> rock = mgr.ObjectById(x610_destroyableRocks[rockIndex])) {
    if (!rock->IsUsingPhazonModel()) {
      rock->UsePhazonModel();
      float hp = rockIndex == x5f0_rockColliders->GetNumCollisionActors() - 1 ? 2.f * x6a4_ : x6a4_;
      CHealthInfo* hInfo = rock->HealthInfo(mgr);
      hInfo->SetHP(hp);
      hInfo->SetKnockbackResistance(2.f);
      hInfo = rockCol->HealthInfo(mgr);
      hInfo->SetHP(hp);
      hInfo->SetKnockbackResistance(2.f);
      CGameLight* light = static_cast<CGameLight*>(mgr.ObjectById(x6c0_rockLights[rockIndex]));
      light->SetActive(true);
      if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::Thermal ||
          (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::Thermal && x7c4_ != 3)) {
        sub801dc444(mgr, GetTranslation(), x6d4_);
      }
      sub801dbc5c(mgr, rock);
      ProcessSoundEvent(x760_, 1.f, 0, 0.1f, 1000.f, 0.16f, 1.f, zeus::skZero3f, GetTranslation(), mgr.GetNextAreaId(),
                        mgr, true);
    }
  }
}

void CThardus::sub801dc444(CStateManager& mgr, const zeus::CVector3f& pos, CAssetId particle) {
  u32 w = x6f4_;
  ++x6f4_;
  std::string particleName = fmt::format(FMT_STRING("ROCK_EFFECT{}-{}"), particle.Value(), w);
  GetModelData()->GetAnimationData()->GetParticleDB().AddAuxiliaryParticleEffect(
      particleName, 0x40, CAuxiliaryParticleData(0, {FOURCC('PART'), particle}, pos, 1.f),
      2.f * GetModelData()->GetScale(), mgr, GetAreaIdAlways(), 0);
}

void CThardus::sub801dbc5c(CStateManager& mgr, CDestroyableRock* rock) {
  if (!x938_) {
    x938_ = true;
    x939_ = false;
    sub801dbbdc(mgr, rock);
  }
}

void CThardus::sub801dbbdc(CStateManager& mgr, CDestroyableRock* rock) {
  if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::Thermal)
    x688_ = true;

  if (x7c4_ == 0 || x7c4_ == 2) {
    x7c4_ = 1;
    x7b8_ = 0.f;
  }

  x928_currentRockId = rock->GetUniqueId();
  x92c_currentRockPos = rock->GetTranslation();
}

void CThardus::UpdateNonDestroyableCollisionActorMaterials(EUpdateMaterialMode mode, EMaterialTypes mat,
                                                           urde::CStateManager& mgr) {
  for (const auto& uid : x634_nonDestroyableActors) {
    if (TCastToPtr<CCollisionActor> col = mgr.ObjectById(uid)) {
      if (mode == EUpdateMaterialMode::Remove)
        col->RemoveMaterial(mat, mgr);
      else if (mode == EUpdateMaterialMode::Add)
        col->AddMaterial(mat, mgr);

      *col->HealthInfo(mgr) = CHealthInfo(1000000.0, 10.f);
    }
  }
}

void CThardus::UpdateExcludeList(const std::unique_ptr<CCollisionActorManager>& colMgr, EUpdateMaterialMode mode,
                                 EMaterialTypes w2, CStateManager& mgr) {
  for (size_t i = 0; i < colMgr->GetNumCollisionActors(); ++i) {
    if (TCastToPtr<CActor> colAct = mgr.ObjectById(colMgr->GetCollisionDescFromIndex(i).GetCollisionActorId())) {
      CMaterialList exclude = colAct->GetMaterialFilter().GetExcludeList();
      if (mode == EUpdateMaterialMode::Remove)
        exclude.Remove(w2);
      else if (mode == EUpdateMaterialMode::Add)
        exclude.Add(w2);

      colAct->SetMaterialFilter(
          CMaterialFilter::MakeIncludeExclude(colAct->GetMaterialFilter().GetIncludeList(), exclude));
    }
  }
}

void CThardus::RenderFlare(const CStateManager& mgr, float t) {
  if (!x91c_flareTexture)
    return;
  if (!m_flareFilter)
    m_flareFilter.emplace(EFilterType::Add, x91c_flareTexture);

  const float scale = 30.f * t;
  zeus::CVector3f offset = scale * CGraphics::g_ViewMatrix.basis[2];
  zeus::CVector3f max = x92c_currentRockPos + (scale * CGraphics::g_ViewMatrix.basis[0]);
  zeus::CVector3f min = x92c_currentRockPos - (scale * CGraphics::g_ViewMatrix.basis[0]);
  CGraphics::SetModelMatrix(zeus::CTransform());
  const std::array<CTexturedQuadFilter::Vert, 4> verts{{
      {{max.x() + offset.x(), max.y() + offset.y(), max.z() + offset.z()}, {0.f, 1.f}},
      {{min.x() + offset.x(), min.y() + offset.y(), min.z() + offset.z()}, {0.f, 0.f}},
      {{max.x() - offset.x(), max.y() - offset.y(), max.z() - offset.z()}, {1.f, 1.f}},
      {{min.x() - offset.x(), min.y() - offset.y(), min.z() - offset.z()}, {1.f, 0.f}},
  }};

  m_flareFilter->drawVerts({t, t}, verts);
}

#if 0
  CGraphics::SetModelMatrix(zeus::CTransform());
  CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::One, ERglBlendFactor::One, ERglLogicOp::Clear);
  CGraphics::SetTevOp(0, CGraphics::sTevPass805a5ebc);
  CGraphics::SetTevOp(1, CGraphics::sTevPass805a564c);
  CGraphics::SetDepthWriteMode(false, ERglEnum::Always, false);
  CGraphics::StreamColor(zeus::CColor(f1, f1));
  CGraphics::StreamBegin(0xa0);
  CGraphics::StreamTexCoord(0.f, 0.f);
  CGraphics::StreamVertex(min + offset);
  CGraphics::StreamTexCoord(1.f, 0.f);
  CGraphics::StreamVertex(min - offset);
  CGraphics::StreamTexCoord(1.f, 1.f);
  CGraphics::StreamVertex(max - offset);
  CGraphics::StreamTexCoord(0.f, 1.f);
  CGraphics::StreamVertex(max + offset);
  CGraphics::StreamEnd();
#endif

zeus::CVector3f CThardus::sub801de550(const CStateManager& mgr) const {
  if (x570_)
    return {};

  return {};
}
} // namespace urde::MP1
