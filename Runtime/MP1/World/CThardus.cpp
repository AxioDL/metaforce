#include "Runtime/MP1/World/CThardus.hpp"

#include <array>

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/Camera/CCameraManager.hpp"
#include "Runtime/Camera/CFirstPersonCamera.hpp"
#include "Runtime/Collision/CCollisionActor.hpp"
#include "Runtime/Collision/CCollisionActorManager.hpp"
#include "Runtime/MP1/CSamusHud.hpp"
#include "Runtime/MP1/World/CIceAttackProjectile.hpp"
#include "Runtime/MP1/World/CThardusRockProjectile.hpp"
#include "Runtime/Weapon/CGameProjectile.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CDestroyableRock.hpp"
#include "Runtime/World/CGameLight.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CRepulsor.hpp"
#include "Runtime/World/CScriptDistanceFog.hpp"
#include "Runtime/World/CScriptWaypoint.hpp"
#include "Runtime/World/CWorld.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

#include <Audio/SFX/IceWorld.h>
#include <Audio/SFX/Thardus.h>
namespace metaforce::MP1 {
namespace {
constexpr std::array<SSphereJointInfo, 7> skDamageableSphereJointInfoList1{{
    {"R_knee", 1.f},
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
    "R_knee"sv,
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
, x6a0_(f3)
, x6a4_(f4)
, x6a8_(f5)
, x6ac_(f6)
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

void CThardus::UpdateRockThermalState(float dt, CStateManager& mgr) {
  if (x7c4_ == 0) {
    x93a_ = false;
    return;
  }

  bool bVar9 = false;
  float dVar15 = 0.f;
  float dVar13 = 1.f;
  if (x928_currentRockId != kInvalidUniqueId) {
    if (TCastToPtr<CActor> rock = mgr.ObjectById(x928_currentRockId)) {
      x92c_currentRockPos = rock->GetTranslation();
    } else {
      x928_currentRockId = kInvalidUniqueId;
    }
  }

  if (!x939_) {
    CGameCamera* cam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
    zeus::CVector3f posDiff = x92c_currentRockPos - cam->GetTranslation();
    zeus::CVector3f camFront = cam->GetTransform().frontVector();
    zeus::CVector3f direction = posDiff.normalized();
    dVar13 = 0.f;
    float dVar11 = direction.dot(camFront);
    if (dVar13 <= dVar11) {
      dVar13 = dVar11 * dVar11;
    }
  }

  if (x7c4_ == 2) {
    dVar15 = -(x7b8_ <= 2.f ? x7b8_ : 2.f) * 0.5f - 1.f; // std::min doesn't quite cut the mustard here
    if (x7b8_ > 2.f) {
      x7c4_ = 0;
      x7b8_ = 0.f;
      x7c0_ = 0.f;
    }
    x7b8_ += dt;
  } else if (x7c4_ == 1) {
    dVar15 = (x7b8_ <= 0.25f ? x7b8_ : 0.25f) / 0.25f;

    if (x7b8_ > 0.25f) {
      x7c4_ = 3;
      x7b8_ = 0.f;
    }
    x7b8_ += dt;
    if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::Thermal) {
      x688_ = true;
    }
  } else if (x7c4_ == 3) {
    dVar15 = 1.f;
    if (x7bc_ < x7b8_ && !x938_) {
      x7c4_ = 2;
      x7b8_ = 0.f;
    }

    x7b8_ += dt;
    if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::Thermal) {
      bVar9 = false;
      x688_ = true;
      if (x938_ && dVar13 > 0.75f) {
        bVar9 = true;
      }
    }
  }

  x7c0_ = dVar15 * dVar13;
  const float mag = 0.8f * (dVar15 * dVar13);
  mgr.SetThermalColdScale2(mgr.GetThermalColdScale2() + mag);
  x50c_baseDamageMag = mag;

  if (x93a_ != bVar9) {
    if (bVar9) {
      CSamusHud::DisplayHudMemo(g_MainStringTable->GetString(18), CHUDMemoParms(5.f, true, false, false));
    }
    x93a_ = bVar9;
  }
}

void CThardus::sub801de9f8(CStateManager& mgr) {
  float dVar5 = mgr.GetActiveRandom()->Float();
  if (!IsLastRock() || dVar5 >= 0.3f) {
    const float local_28 = std::max(0.f, dVar5 - 0.19999999f);
    if (local_28 > 0.8f) {
      x5c4_ = 2;
    } else if (local_28 >= 0.4f) {
      x5c4_ = 1;
    } else {
      x5c4_ = 0;
    }
    ++x574_;
    x944_ = 0.3f;
  } else {
    x93b_ = true;
  }
}

void CThardus::sub801dd608(CStateManager& mgr) {
  zeus::CVector3f scale = GetModelData()->GetScale();
  CAnimData* animData = GetModelData()->GetAnimationData();
  for (size_t i = 0; i < x610_destroyableRocks.size(); ++i) {
    zeus::CTransform xf =
        GetTransform() * (zeus::CTransform::Scale(scale) * animData->GetLocatorTransform(skRockJoints[i], nullptr));
    if (TCastToPtr<CActor> act = mgr.ObjectById(x610_destroyableRocks[i])) {
      act->SetTransform(xf);
    }
    if (TCastToPtr<CGameLight> gl = mgr.ObjectById(x6c0_rockLights[i])) {
      gl->SetTransform(xf);
    }
  }
}
void CThardus::sub801dcfa4(CStateManager& mgr) {
  for (size_t j = 0; j < x5f0_rockColliders->GetNumCollisionActors(); ++j) {
    const auto& jInfo = x5f0_rockColliders->GetCollisionDescFromIndex(j);
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(jInfo.GetCollisionActorId())) {
      if (!colAct->GetActive()) {
        continue;
      }
      TUniqueId rockId = x610_destroyableRocks[j];
      if (auto* rock = static_cast<CDestroyableRock*>(mgr.ObjectById(rockId))) {
        if (x909_) {
          *rock->HealthInfo(mgr) = CHealthInfo(x90c_rockHealths[j], 0.f);
        }
        if (j == x648_currentRock && !x93d_) {
          colAct->SetDamageVulnerability(*rock->GetDamageVulnerability());
          rock->SetThermalMag(0.8f);
        } else {
          colAct->SetDamageVulnerability(CDamageVulnerability::ImmuneVulnerabilty());
          rock->SetThermalMag(0.f);
        }

        CHealthInfo* hInfo = colAct->HealthInfo(mgr);
        if (hInfo != nullptr) {
          if (hInfo->GetHP() > 0.f) {
            *rock->HealthInfo(mgr) = *hInfo;
            if (!x909_) {
              x90c_rockHealths[j] = hInfo->GetHP();
            }
          } else if (!rock->IsUsingPhazonModel()) {
            BreakRock(mgr, j);
            DoFaint(mgr);
          } else {
            rock->SetActive(false);
            colAct->SetActive(false);
            mgr.ObjectById(x6c0_rockLights[j])->SetActive(false);
            ++x648_currentRock;
            DoFlinch(mgr);
            const bool isThermalActive = mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::Thermal;
            if (isThermalActive || (!isThermalActive && x7c4_ != 3)) {
              SetRockParticle(mgr, GetTranslation(), x6d8_);
            }
            ProcessSoundEvent(x758_, 1.f, 0, 0.1f, 1000.f, 0.16f, 1.f, zeus::skZero3f, GetTranslation(),
                              mgr.GetNextAreaId(), mgr, true);

            if (IsLastRock() && !x8f0_) {
              DoDoubleSnap(mgr);
            }
            sub801dbc40();
          }
        }
      }
    }
  }
}

void CThardus::Think(float dt, CStateManager& mgr) {
  if (!GetActive() || !x450_bodyController->GetActive()) {
    return;
  }

  if (!x91c_flareTexture) {
    x91c_flareTexture.Lock();
  }

  if (x7c8_) {
    float fVar2 = 10.f * GetModelData()->GetScale().x();
    zeus::CVector3f diff = mgr.GetPlayer().GetTranslation() - x7cc_;
    if (diff.magSquared() < fVar2 * fVar2) {
      mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer().GetUniqueId(), GetUniqueId(),
                      CDamageInfo(CWeaponMode(EWeaponType::AI), 0.f, 0.f, 10.f), CMaterialFilter::skPassEverything,
                      diff.normalized());
      x688_ = true;
      x7c8_ = false;
      x7cc_.zeroOut();
    }
  }

  UpdateRockThermalState(dt, mgr);

  if (!IsLastRock()) {
    // NOTE: (phil), yes this is what's actually happening
#if 0
    if (x648_currentRock < x610_destroyableRocks.size() - 2) {
      x690_ = 1.f;
    } else {
      x690_ = 1.f;
    }
#endif
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

  if (x610_destroyableRocks.size() <= x648_currentRock) {
    Death(mgr, zeus::skZero3f, EScriptObjectState::DeathRattle);
  }

  if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::Thermal) {
    x402_29_drawParticles = false;
    UpdateNonDestroyableCollisionActorMaterials(EUpdateMaterialMode::Remove, EMaterialTypes::ProjectilePassthrough,
                                                mgr);
    for (size_t i = 0; i < x610_destroyableRocks.size(); ++i) {
      if (auto* act = static_cast<CActor*>(mgr.ObjectById(x610_destroyableRocks[i]))) {
        if (x648_currentRock == i && !x688_ && !x93c_ && !x909_ && !x93d_) {
          act->AddMaterial(EMaterialTypes::Orbit, mgr);
          act->AddMaterial(EMaterialTypes::Target, mgr);
        } else {
          act->RemoveMaterial(EMaterialTypes::Orbit, mgr);
          act->RemoveMaterial(EMaterialTypes::Target, mgr);
        }
      }
    }
    if (x688_) {
      x688_ = false;
    }

  } else {
    x402_29_drawParticles = true;
    UpdateNonDestroyableCollisionActorMaterials(EUpdateMaterialMode::Add, EMaterialTypes::ProjectilePassthrough, mgr);
    for (size_t i = 0; i < x610_destroyableRocks.size(); ++i) {
      if (auto* act = static_cast<CActor*>(mgr.ObjectById(x610_destroyableRocks[i]))) {
        if (!x688_ && !x93c_ && !x909_ && !x93d_) {
          bool found = act->GetName().find("Neck_1"sv) != std::string::npos;
          if (!found || !x6b0_destroyedRocks[x648_currentRock] ||
              x648_currentRock == x610_destroyableRocks.size() - 1) {
            if (!x6b0_destroyedRocks[i]) {
              if (!found || x6b0_destroyedRocks[i]) {
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

  UpdateHealthInfo(mgr);
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
    if (!x5c8_heardPlayer) {
      x5c8_heardPlayer = true;
    }
    break;
  }
  case EScriptObjectMessage::Touched: {
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(uid)) {
      if (TCastToPtr<CPlayer> pl = mgr.ObjectById(colAct->GetLastTouchedObject())) {
        if (x420_curDamageRemTime > 0.f) {
          break;
        }
        u32 rand = static_cast<u32>(mgr.GetActiveRandom()->Next());
        float damageMult = 1.f;
        zeus::CVector3f knockBack = zeus::skForward;
        if (x644_ == 1) {
          damageMult = 2.f;
          knockBack = (rand % 2) != 0u ? zeus::skRight : zeus::skLeft;
        }

        if (mgr.GetPlayer().GetFrozenState()) {
          mgr.GetPlayer().UnFreeze(mgr);
        }

        knockBack = GetTransform().buildMatrix3f() * knockBack;
        CDamageInfo dInfo = GetContactDamage();
        dInfo.SetDamage(damageMult * dInfo.GetDamage());
        mgr.ApplyDamage(GetUniqueId(), pl->GetUniqueId(), GetUniqueId(), dInfo,
                        CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}),
                        x644_ == 1 ? knockBack : zeus::skZero3f);
        x420_curDamageRemTime = x424_damageWaitTime;
      } else if (TCastToConstPtr<CBomb>(mgr.GetObjectById(colAct->GetLastTouchedObject()))) {
        if (x644_ == 1 && x93c_) {
          BreakRock(mgr, x648_currentRock);
        }
      }
    }

    break;
  }
  case EScriptObjectMessage::Registered: {
    x610_destroyableRocks.reserve(x5cc_.size());
    x6b0_destroyedRocks.reserve(x5cc_.size());
    x6c0_rockLights.reserve(x5cc_.size());
    x90c_rockHealths.reserve(x5cc_.size());
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
      x6b0_destroyedRocks.push_back(false);
      TUniqueId lightId = mgr.AllocateUniqueId();
      auto* gl = new CGameLight(lightId, GetAreaIdAlways(), false, ""sv, {}, GetUniqueId(),
                                CLight::BuildPoint({}, zeus::skBlue), 0, 0, 0.f);
      gl->SetActive(false);
      mgr.AddObject(gl);
      x6c0_rockLights.push_back(lightId);
      x90c_rockHealths.push_back(health);
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
    if (x94c_initialized) {
      break;
    }
    x94c_initialized = true;
    x764_startTransform = GetTransform();

    for (const SConnection& conn : GetConnectionList()) {
      TUniqueId connId = mgr.GetIdForScript(conn.x8_objId);
      if (connId == kInvalidUniqueId) {
        continue;
      }

      if (conn.x0_state == EScriptObjectState::Patrol) {
        if (TCastToPtr<CScriptWaypoint> wp = mgr.ObjectById(connId)) {
          rstl::reserved_vector<TUniqueId, 16> wpIds;
          GatherWaypoints(wp, mgr, wpIds);
          x578_waypoints.push_back(wpIds);
        } else if (CPatterned* p = CPatterned::CastTo<CThardusRockProjectile>(mgr.ObjectById(connId))) {
          x5fc_projectileId = connId;
          x60c_projectileEditorId = conn.x8_objId;
          p->SetActive(false);
        } else if (TCastToConstPtr<CRepulsor>(mgr.GetObjectById(connId))) {
          x664_repulsors.push_back(connId);
        } else if (TCastToConstPtr<CScriptDistanceFog>(mgr.GetObjectById(connId))) {
          x64c_fog = connId;
        }
      } else if (conn.x0_state == EScriptObjectState::Zero) {
        if (TCastToPtr<CScriptWaypoint> wp = mgr.ObjectById(connId)) {
          x8f4_waypoints.push_back(connId);
          wp->SetActive(false);
        }
      } else if (conn.x0_state == EScriptObjectState::Dead) {
        if (TCastToConstPtr<CScriptTimer>(mgr.GetObjectById(connId))) {
          x7a8_timers.push_back(connId);
        }
      }
    }
    x7f0_pathFindSearch.SetArea(mgr.GetWorld()->GetAreaAlways(GetAreaIdAlways())->GetPostConstructed()->x10bc_pathArea);
    break;
  }
  case EScriptObjectMessage::Damage: {
    if (TCastToConstPtr<CCollisionActor> colAct = mgr.GetObjectById(uid)) {
      TUniqueId lastTouchedObj = colAct->GetLastTouchedObject();
      TUniqueId targetRock = kInvalidUniqueId;

      for (size_t i = 0; i < x5f0_rockColliders->GetNumCollisionActors(); ++i) {
        const CJointCollisionDescription& desc = x5f0_rockColliders->GetCollisionDescFromIndex(i);
        if (desc.GetCollisionActorId() == uid) {
          targetRock = x610_destroyableRocks[i];
          break;
        }
      }

      if (targetRock == kInvalidUniqueId) {
        break;
      }
      if (auto* rock = static_cast<CDestroyableRock*>(mgr.ObjectById(targetRock))) {
        if (TCastToConstPtr<CGameProjectile> proj = mgr.GetObjectById(lastTouchedObj)) {
          if (GetBodyController()->GetBodyStateInfo().GetCurrentAdditiveStateId() !=
                  pas::EAnimationState::AdditiveReaction &&
              rock->Get_x324() <= 0.f) {
            GetBodyController()->GetCommandMgr().DeliverCmd(
                CBCAdditiveReactionCmd(pas::EAdditiveReactionType::Five, 1.f, false));
          }

          rock->TakeDamage(zeus::skZero3f, 0.f);
          const bool thermalInactive = mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::Thermal;
          if (thermalInactive || x7c4_ != 3) {
            SetRockParticle(mgr, proj->GetTranslation(), x6d0_);
          }
          if (!rock->IsUsingPhazonModel()) {
            ProcessSoundEvent(x75c_, 1.f, 0, 0.1f, 1000.f, 0.16f, 1.f, zeus::skZero3f, rock->GetTranslation(),
                              mgr.GetNextAreaId(), mgr, true);
          } else {
            ProcessSoundEvent(SFXsfx0AC0, 1.f, 0, 0.1f, 1000.f, 0.16f, 1.f, zeus::skZero3f, rock->GetTranslation(),
                              mgr.GetNextAreaId(), mgr, false);
          }
        }
      }
    }
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

void CThardus::Touch(CActor& act, CStateManager& mgr) {
  // Intentionally empty
}

zeus::CVector3f CThardus::GetOrbitPosition(const CStateManager& mgr) const { return GetAimPosition(mgr, 0.f); }

zeus::CVector3f CThardus::GetAimPosition(const CStateManager& mgr, float dt) const {
  return GetLctrTransform(x93c_ ? "center_LCTR"sv : "Neck_1"sv).origin;
}
zeus::CAABox CThardus::GetSortingBounds(const CStateManager& mgr) const {
  zeus::CVector3f extents = 0.15f * (x9c_renderBounds.max - x9c_renderBounds.min);
  return zeus::CAABox(x9c_renderBounds.min - extents, x9c_renderBounds.max + extents);
}

void CThardus::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  switch (type) {
  case EUserEventType::Projectile: {
    zeus::CTransform wristXf = GetLctrTransform("L_wrist"sv);
    CRayCastResult res = mgr.RayStaticIntersection(wristXf.origin, zeus::skDown, 100.f,
                                                   CMaterialFilter::MakeInclude(EMaterialTypes::Solid));
    zeus::CTransform xf = zeus::lookAt(res.GetPoint() + zeus::CVector3f{0.f, 0.f, 1.f}, GetTranslation());
    xf.rotateLocalZ(zeus::degToRad(mgr.GetActiveRandom()->Range(-5.f, 5.f)));
    mgr.AddObject(new CIceAttackProjectile(
        g_SimplePool->GetObj({SBIG('PART'), x600_}), g_SimplePool->GetObj({SBIG('PART'), x604_}),
        g_SimplePool->GetObj({SBIG('PART'), x608_}), mgr.AllocateUniqueId(), GetAreaIdAlways(),
        mgr.GetPlayer().GetUniqueId(), true, xf, CDamageInfo(CWeaponMode::Ice(), 6.f, 0.f, 0.f), zeus::CAABox(0.f, 1.f),
        x6ac_, zeus::degToRad(42.f), x6e8_, x6ec_, SFXsfx0AAD, x6f0_));
    break;
  }
  case EUserEventType::LoopedSoundStop:
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
    break;
  case EUserEventType::AlignTargetPos: {
    SetState(2, mgr);
    break;
  }
  case EUserEventType::Delete:
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
    break;
  case EUserEventType::DamageOn: {
    x7c8_ = true;
    x7cc_ = (zeus::CTransform::Scale(GetModelData()->GetScale()) *
             GetModelData()->GetAnimationData()->GetLocatorTransform("R_ankle"sv, nullptr))
                .origin;
    break;
  }
  case EUserEventType::DamageOff: {
    x7c8_ = false;
    x7cc_ = zeus::skZero3f;
    break;
  }
  case EUserEventType::Landing: {
    x93c_ = false;
    break;
  }
  case EUserEventType::TakeOff: {
    s32 rnd = mgr.GetActiveRandom()->Next();
    std::vector<u32> inRangeWaypoints;
    inRangeWaypoints.reserve(x8f4_waypoints.size());

    zeus::CVector3f plPos = mgr.GetPlayer().GetTranslation();

    for (size_t i = 0; i < x8f4_waypoints.size(); ++i) {
      if (TCastToPtr<CScriptWaypoint> wp = mgr.ObjectById(x8f4_waypoints[i])) {
        if ((wp->GetTranslation() - plPos).magSquared() > 10.f) {
          inRangeWaypoints.push_back(i);
        }
      }
    }

    for (size_t i = 0; i < (rnd & 1) + 2; ++i) {
      if (TCastToPtr<CScriptWaypoint> wp =
              mgr.ObjectById(x8f4_waypoints[mgr.GetActiveRandom()->Next() % inRangeWaypoints.size()])) {
        wp->SetActive(true);
        SendScriptMsgs(EScriptObjectState::Zero, mgr, EScriptObjectMessage::None);
        wp->SetActive(false);
      }
    }
    break;
  }
  case EUserEventType::FadeIn: {
    break;
  }
  case EUserEventType::FadeOut: {
    if (x644_ == 1) {
      x93c_ = true;
      x688_ = true;
    }
    break;
  }
  case EUserEventType::ScreenShake:
    ApplyCameraShake(1.25f, 125.f, 1.f, mgr, GetTranslation());
    [[fallthrough]];
  default:
    break;
  }
}

void CThardus::Patrol(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x658_ = -1;
    x950_ = mgr.GetPlayer().GetTranslation();
  } else if (msg == EStateMsg::Update) {
    if (!ShouldTurn(mgr, 0.f)) {
      return;
    }

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
  if (msg == EStateMsg::Activate) {
    x2e0_destPos = sub801de550(mgr);
    x7e4_ = x7d8_ = x2e0_destPos;
    CPatterned::PathFind(mgr, EStateMsg::Activate, arg);
  } else if (msg == EStateMsg::Update) {
    if ((GetTranslation().toVec2f() - x7e4_.toVec2f()).magnitude() < 10.f) {
      x2e0_destPos = sub801de434(mgr);
      x7d8_ = x2e0_destPos;
      x7e4_ = x7d8_;
      CPatterned::PathFind(mgr, EStateMsg::Activate, arg);
      ++x660_;
    }
    x650_ = GetTargetVector(arg, mgr);
  } else if (msg == EStateMsg::Deactivate) {
    x8d4_ = false;
  }
}

void CThardus::TargetPatrol(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x5ec_stateProg = 0;
    if (x95e_) {
      return;
    }

    mgr.SetBossParams(GetUniqueId(), GetHealthInfo(mgr)->GetHP(), 88);
    x95e_ = true;
  } else if (msg == EStateMsg::Update) {
    if (x5ec_stateProg == 0) {
      if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Taunt) {
        x5ec_stateProg = 2;
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCTauntCmd(pas::ETauntType::One));
      }
    } else if (x5ec_stateProg == 2 &&
               x450_bodyController->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Taunt) {
      x5ec_stateProg = 3;
    }
  }
}

void CThardus::Generate(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x5ec_stateProg = 0;
  } else if (msg == EStateMsg::Update) {
    if (x5ec_stateProg == 0) {
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Getup) {
        x5ec_stateProg = 2;
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCGetupCmd(pas::EGetupType::Zero));
      }
    } else if (x5ec_stateProg == 2 && x450_bodyController->GetCurrentStateId() != pas::EAnimationState::Getup) {
      x5ec_stateProg = 3;
    }
  } else if (msg == EStateMsg::Deactivate) {
    x93d_ = false;
  }
}

void CThardus::Attack(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x5ec_stateProg = 0;
    ++x570_;
    x5ec_stateProg = 0;
  } else if (msg == EStateMsg::Update) {
    if (x5ec_stateProg == 0) {
      if (GetBodyController()->GetCurrentStateId() != pas::EAnimationState::MeleeAttack) {
        if (mgr.GetActiveRandom()->Float() <= 0.5f) {
          GetBodyController()->GetCommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::ESeverity::One));
        } else {
          GetBodyController()->GetCommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::ESeverity::Zero));
        }
        ++x570_;
      } else {
        x5ec_stateProg = 2;
      }
    } else if (x5ec_stateProg == 2 && GetBodyController()->GetCurrentStateId() != pas::EAnimationState::MeleeAttack) {
      x5ec_stateProg = 3;
    }
  }
}

void CThardus::LoopedAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x658_ = 0;
    x660_ = 0;
    x570_ = 0;
    x574_ = 0;
    sub801dec80();
    x93b_ = false;
    x5c4_ = -1;
  } else if (msg == EStateMsg::Update) {
    const zeus::CVector3f thisPos = GetTranslation();
    if (x658_ == 1) {
      const zeus::CVector3f offset = thisPos + zeus::CVector3f{0.f, 0.f, 10.f};
      CRayCastResult result = mgr.RayStaticIntersection(
          offset, zeus::CQuaternion(GetTransform().buildMatrix3f()).toTransform() * zeus::CVector3f{0.f, 1.f, 0.f},
          100.f, CMaterialFilter::MakeInclude({EMaterialTypes::Wall, EMaterialTypes::Floor, EMaterialTypes::Ceiling}));
      if (result.IsInvalid()) {
        zeus::CVector2f vec = GetSteeringVector(mgr);
        if (vec != zeus::skZero2f) {
          x650_ = vec;
        }
      } else {
        x8d8_ = result.GetPoint();
        x8e4_ = offset;
        if ((result.GetPoint() - offset).magnitude() < 20.f) {
          x658_ = 2;
          x8d4_ = true;
        }
      }
    } else if (x658_ == 0) {
      zeus::CVector3f dir = (mgr.GetPlayer().GetTranslation() - GetTranslation()).normalized();
      zeus::CVector2f vec = GetSteeringVector(mgr);
      if (vec != zeus::skZero2f) {
        x650_ = vec;
      } else {
        x650_ = dir.toVec2f().normalized();
      }

      if (dir.magnitude() < x698_) {
        x658_ = 1;
      }
    }
    GetBodyController()->GetCommandMgr().DeliverCmd(CBodyStateCmd{EBodyStateCmd::MaintainVelocity});
    GetBodyController()->GetCommandMgr().DeliverCmd(CBCLocomotionCmd{zeus::CVector3f(x650_), zeus::skZero3f, 1.f});
  }
}

void CThardus::DoubleSnap(CStateManager& mgr, EStateMsg msg, float arg) {
  // Intentionally empty
}

void CThardus::Shuffle(CStateManager& mgr, EStateMsg msg, float arg) {
  // Intentionally empty
}

void CThardus::GetUp(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg != EStateMsg::Activate) {
    return;
  }
  RemoveMaterial(EMaterialTypes::RadarObject, mgr);
}

void CThardus::Taunt(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x5ec_stateProg = 0;
  } else if (msg == EStateMsg::Update) {
    if (x5ec_stateProg == 0) {
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Taunt) {
        x5ec_stateProg = 2;
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCTauntCmd(pas::ETauntType::One));
      }
    } else if (x5ec_stateProg == 2 && x450_bodyController->GetCurrentStateId() != pas::EAnimationState::Taunt) {
      x5ec_stateProg = 3;
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
    x5ec_stateProg = 0;
  } else if (msg == EStateMsg::Update) {
    if (x5ec_stateProg == 1) {
      return;
    }

    if (x5ec_stateProg == 0) {
      if (GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::ProjectileAttack) {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCProjectileAttackCmd(pas::ESeverity::Zero, {}, false));
        x5ec_stateProg = 0;
      } else {
        x5ec_stateProg = 2;
      }
    } else if (x5ec_stateProg == 2 &&
               GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::ProjectileAttack) {
      x5ec_stateProg = 3;
    }
  }
}

void CThardus::Flinch(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x5ec_stateProg = 0;
    for (TUniqueId uid : x798_) {
      if (auto* rock = CPatterned::CastTo<CThardusRockProjectile>(mgr.ObjectById(uid))) {
        rock->sub80203d58();
      }
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
    default:
      break;
    }

    if (x5ec_stateProg == 0) {
      if (GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::KnockBack) {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCKnockBackCmd({}, severity));
      } else {
        x5ec_stateProg = 2;
      }
    } else if (x5ec_stateProg == 2 &&
               GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::KnockBack) {
      x5ec_stateProg = 3;
    }
  }
}

void CThardus::TelegraphAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x5ec_stateProg = 0;
  } else if (msg == EStateMsg::Update) {
    if (x5ec_stateProg == 0) {
      if (GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::ProjectileAttack) {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCProjectileAttackCmd(pas::ESeverity::One, {}, false));
        x5ec_stateProg = 0;
      } else {
        x5ec_stateProg = 2;
      }
    } else if (x5ec_stateProg == 2 &&
               GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::ProjectileAttack) {
      x5ec_stateProg = 3;
    }
  }
}

void CThardus::Explode(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x5ec_stateProg = 0;
    CSfxManager::SfxStop(x904_);
    x909_ = true;
    x93d_ = true;
    x909_ = true;
    SendScriptMsgs(EScriptObjectState::Arrived, mgr, EScriptObjectMessage::None);
  } else if (msg == EStateMsg::Update) {
    if (x5ec_stateProg == 0) {
      if (GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Step) {
        GetBodyController()->GetCommandMgr().DeliverCmd(
            CBCStepCmd(pas::EStepDirection::Forward, pas::EStepType::Dodge));
      } else {
        x5ec_stateProg = 2;
      }
    } else if (x5ec_stateProg == 2 &&
               GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Step) {
      x5ec_stateProg = 3;
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
    if (x610_destroyableRocks.size() - 2 <= x648_currentRock) {
      x690_ = 1.1f;
    }
    AddMaterial(EMaterialTypes::RadarObject, mgr);
  } else if (msg == EStateMsg::Deactivate) {
    x690_ = 1.f;
  }
}

void CThardus::Enraged(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x5ec_stateProg = 0;
    x688_ = true;
    x908_ = true;
  } else if (msg == EStateMsg::Update) {
    if (x5ec_stateProg == 0) {
      if (GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Taunt) {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCTauntCmd(pas::ETauntType::Zero));
      } else {
        x5ec_stateProg = 2;
      }
    } else if (x5ec_stateProg == 2 &&
               GetBodyController()->GetBodyStateInfo().GetCurrentStateId() != pas::EAnimationState::Taunt) {
      x5ec_stateProg = 3;
    }
  } else if (msg == EStateMsg::Deactivate) {
    x908_ = false;
  }
}

void CThardus::Growth(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x5ec_stateProg = 0;
    x904_ = CSfxManager::SfxStart(SFXsfx07AD, 1.f, 0.f, false, 0x7f, true, GetAreaIdAlways());
  } else if (msg == EStateMsg::Update) {
    if (x5ec_stateProg == 0) {
      if (GetBodyController()->GetCurrentStateId() != pas::EAnimationState::Step) {
        GetBodyController()->GetCommandMgr().DeliverCmd(
            CBCStepCmd(pas::EStepDirection::Forward, pas::EStepType::BreakDodge));
      } else {
        x5ec_stateProg = 2;
      }
    } else if (x5ec_stateProg == 2 && GetBodyController()->GetCurrentStateId() != pas::EAnimationState::Step) {
      x5ec_stateProg = 3;
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
    x5ec_stateProg = 0;
    x93c_ = false;
    SetState(-1, mgr);
    for (TUniqueId uid : x798_) {
      if (auto* rock = CPatterned::CastTo<CThardusRockProjectile>(mgr.ObjectById(uid))) {
        rock->sub80203d58();
      }
    }
    x94d_ = true;
  } else if (msg == EStateMsg::Update) {
    if (x5ec_stateProg == 0) {
      if (GetBodyController()->GetCurrentStateId() != pas::EAnimationState::KnockBack) {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCKnockBackCmd({}, pas::ESeverity::Six));
      } else {
        x5ec_stateProg = 2;
      }
    } else if (x5ec_stateProg == 2 && GetBodyController()->GetCurrentStateId() != pas::EAnimationState::KnockBack) {
      x5ec_stateProg = 3;
    }
  }
}

bool CThardus::PathFound(CStateManager& mgr, float arg) { return x8d4_; }
bool CThardus::InRange(CStateManager& mgr, float arg) {
  return (mgr.GetPlayer().GetTranslation().toVec2f() - GetTranslation().toVec2f()).magnitude() <
         10.f * GetModelData()->GetScale().x();
}

bool CThardus::PatternOver(CStateManager& mgr, float arg) { return x570_ != 0 || x93b_; }
bool CThardus::AnimOver(CStateManager& mgr, float arg) { return x5ec_stateProg == 3; }
bool CThardus::InPosition(CStateManager& mgr, float arg) { return x660_ > 3; }
bool CThardus::ShouldTurn(CStateManager& mgr, float arg) {
  return std::fabs(zeus::CVector2f::getAngleDiff(GetTransform().frontVector().toVec2f(),
                                                 mgr.GetPlayer().GetTranslation().toVec2f() -
                                                     GetTranslation().toVec2f())) > zeus::degToRad(30.f);
}
bool CThardus::HitSomething(CStateManager& mgr, float arg) { return mgr.GetPlayer().GetFrozenState(); }

void CThardus::GatherWaypoints(metaforce::CScriptWaypoint* wp, metaforce::CStateManager& mgr,
                               rstl::reserved_vector<TUniqueId, 16>& uids) {
  if (uids.size() < uids.capacity() && wp->GetActive()) {
    uids.push_back(wp->GetUniqueId());
    wp->SetActive(false);
    for (const SConnection& conn : wp->GetConnectionList()) {
      TUniqueId uid = mgr.GetIdForScript(conn.x8_objId);
      if (TCastToPtr<CScriptWaypoint> wp2 = mgr.ObjectById(uid)) {
        GatherWaypoints(wp2, mgr, uids);
      }
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
    if (auto* act = static_cast<CActor*>(mgr.ObjectById(colDesc.GetCollisionActorId()))) {
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
  FindNonDestroyableActors(x5f4_);
  FindNonDestroyableActors(x5f8_);
  for (size_t i = 0; i < x5f0_rockColliders->GetNumCollisionActors(); ++i) {
    const auto& colDesc = x5f0_rockColliders->GetCollisionDescFromIndex(i);
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(colDesc.GetCollisionActorId())) {
      if (auto* rock = static_cast<CDestroyableRock*>(mgr.ObjectById(x610_destroyableRocks[i]))) {
        if (i == 0) {
          colAct->SetDamageVulnerability(*rock->GetDamageVulnerability());
          rock->SetThermalMag(0.8f);
        } else {
          colAct->SetDamageVulnerability(CDamageVulnerability::ImmuneVulnerabilty());
          rock->SetThermalMag(0.f);
        }

        *colAct->HealthInfo(mgr) = *rock->HealthInfo(mgr);
      }
    }
  }
}

void CThardus::FindNonDestroyableActors(const std::unique_ptr<CCollisionActorManager>& colMgr) {
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

    if (!foundBone) {
      x634_nonDestroyableActors.push_back(uid);
    }
  }
}

void CThardus::BreakRock(CStateManager& mgr, u32 rockIndex) {
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
      x6b0_destroyedRocks[rockIndex] = true;
      rock->SetThermalMag(1.5f);
      auto* light = static_cast<CGameLight*>(mgr.ObjectById(x6c0_rockLights[rockIndex]));
      light->SetActive(true);
      if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::Thermal ||
          (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::Thermal && x7c4_ != 3)) {
        SetRockParticle(mgr, GetTranslation(), x6d4_);
      }
      x90c_rockHealths[rockIndex] = hp;
      sub801dbc5c(mgr, rock);
      ProcessSoundEvent(x760_, 1.f, 0, 0.1f, 1000.f, 0.16f, 1.f, zeus::skZero3f, GetTranslation(), mgr.GetNextAreaId(),
                        mgr, true);
    }
  }
}

void CThardus::SetRockParticle(CStateManager& mgr, const zeus::CVector3f& pos, CAssetId particle) {
  u32 w = x6f4_;
  ++x6f4_;
  std::string particleName = fmt::format(FMT_STRING("ROCK_EFFECT{}-{}"), particle.Value(), w);
  GetModelData()->GetAnimationData()->GetParticleDB().AddAuxiliaryParticleEffect(
      particleName, 0x40, CAuxiliaryParticleData(0, {FOURCC('PART'), particle}, pos, 1.f),
      2.f * GetModelData()->GetScale(), mgr, GetAreaIdAlways(), 0);
}

void CThardus::sub801dbc5c(CStateManager& mgr, CDestroyableRock* rock) {
  if (x938_) {
    return;
  }

  x938_ = true;
  x939_ = false;
  sub801dbbdc(mgr, rock);
}

void CThardus::sub801dbbdc(CStateManager& mgr, CDestroyableRock* rock) {
  if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::Thermal) {
    x688_ = true;
  }

  if (x7c4_ == 0 || x7c4_ == 2) {
    x7c4_ = 1;
    x7b8_ = 0.f;
  }

  x928_currentRockId = rock->GetUniqueId();
  x92c_currentRockPos = rock->GetTranslation();
}

void CThardus::UpdateNonDestroyableCollisionActorMaterials(EUpdateMaterialMode mode, EMaterialTypes mat,
                                                           metaforce::CStateManager& mgr) {
  for (const auto& uid : x634_nonDestroyableActors) {
    if (TCastToPtr<CCollisionActor> col = mgr.ObjectById(uid)) {
      if (mode == EUpdateMaterialMode::Remove) {
        col->RemoveMaterial(mat, mgr);
      } else if (mode == EUpdateMaterialMode::Add) {
        col->AddMaterial(mat, mgr);
      }

      *col->HealthInfo(mgr) = CHealthInfo(1000000.0f, 10.f);
    }
  }
}

void CThardus::UpdateExcludeList(const std::unique_ptr<CCollisionActorManager>& colMgr, EUpdateMaterialMode mode,
                                 EMaterialTypes w2, CStateManager& mgr) {
  for (size_t i = 0; i < colMgr->GetNumCollisionActors(); ++i) {
    if (TCastToPtr<CActor> colAct = mgr.ObjectById(colMgr->GetCollisionDescFromIndex(i).GetCollisionActorId())) {
      CMaterialList exclude = colAct->GetMaterialFilter().GetExcludeList();
      if (mode == EUpdateMaterialMode::Remove) {
        exclude.Remove(w2);
      } else if (mode == EUpdateMaterialMode::Add) {
        exclude.Add(w2);
      }

      colAct->SetMaterialFilter(
          CMaterialFilter::MakeIncludeExclude(colAct->GetMaterialFilter().GetIncludeList(), exclude));
    }
  }
}

void CThardus::RenderFlare(const CStateManager& mgr, float t) {
  if (!x91c_flareTexture) {
    return;
  }
  x91c_flareTexture->Load(GX::TEXMAP0, EClampMode::Repeat);

  const float scale = 30.f * t;
  zeus::CVector3f offset = scale * CGraphics::g_ViewMatrix.basis[2];
  zeus::CVector3f max = x92c_currentRockPos + (scale * CGraphics::g_ViewMatrix.basis[0]);
  zeus::CVector3f min = x92c_currentRockPos - (scale * CGraphics::g_ViewMatrix.basis[0]);
  CGraphics::SetModelMatrix({});
  CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::One, ERglBlendFactor::One, ERglLogicOp::Clear);
  CGraphics::SetTevOp(ERglTevStage::Stage0, CTevCombiners::sTevPass805a5ebc);
  CGraphics::SetTevOp(ERglTevStage::Stage1, CTevCombiners::skPassThru);
  CGraphics::SetDepthWriteMode(false, ERglEnum::Always, false);
  CGraphics::StreamColor({t, t});
  CGraphics::StreamBegin(GX::TRIANGLEFAN);
  CGraphics::StreamTexcoord(0.f, 0.f);
  CGraphics::StreamVertex(min + offset);
  CGraphics::StreamTexcoord(1.f, 0.f);
  CGraphics::StreamVertex(min - offset);
  CGraphics::StreamTexcoord(1.f, 1.f);
  CGraphics::StreamVertex(max - offset);
  CGraphics::StreamTexcoord(0.f, 1.f);
  CGraphics::StreamVertex(max + offset);
  CGraphics::StreamEnd();
}

zeus::CVector3f CThardus::sub801de550(CStateManager& mgr) {

  s8 wpIdx = -1;
  s8 pathIdx = -1;
  if (!x578_waypoints.empty()) {
    zeus::CVector2f thisPos = GetTranslation().toVec2f();
    std::vector<u32> unkVec;
    unkVec.reserve(x578_waypoints.size());
    for (const auto& path : x578_waypoints) {
      float maxDist = 1000000.f;
      u32 lastIdx = 0;
      for (size_t i = 0; i < path.size(); ++i) {
        TCastToConstPtr<CScriptWaypoint> wp = mgr.GetObjectById(path[i]);
        const float curDist = (wp->GetTranslation().toVec2f() - thisPos).magnitude();
        if (curDist < maxDist) {
          lastIdx = i;
          maxDist = curDist;
        }
      }

      unkVec.push_back(lastIdx);
    }
    // zeus::CVector2f plVec = mgr.GetPlayer().GetTranslation().toVec2f();

    float maxDist = 0.f;
    float curDist = 0.f;

    for (size_t i = 0; i < x578_waypoints.size(); ++i) {
      TCastToConstPtr<CScriptWaypoint> wp1 = mgr.GetObjectById(x578_waypoints[i][unkVec[i]]);
      TCastToConstPtr<CScriptWaypoint> wp2 = mgr.GetObjectById(wp1->NextWaypoint(mgr));
      const float wp1Dist = (wp1->GetTranslation().toVec2f() - mgr.GetPlayer().GetTranslation().toVec2f()).magnitude();
      const float wp2Dist = (wp2->GetTranslation().toVec2f() - mgr.GetPlayer().GetTranslation().toVec2f()).magnitude();
      if (curDist < wp1Dist && wp1Dist <= wp2Dist) {
        curDist = wp1Dist;
        pathIdx = i;
      }

      if (maxDist < wp1Dist) {
        maxDist = wp1Dist;
        wpIdx = i;
      }
    }

    if (pathIdx == -1) {
      pathIdx = wpIdx;
    }
  }

  if (pathIdx == -1 || wpIdx == -1) {
    return {};
  }

  x8f1_curPatrolPath = pathIdx;
  x8f2_curPatrolPathWaypoint = wpIdx;
  return TCastToConstPtr<CScriptWaypoint>(mgr.GetObjectById(x578_waypoints[pathIdx][wpIdx]))->GetTranslation();
}

zeus::CVector3f CThardus::sub801de434(CStateManager& mgr) {
  if (x8f1_curPatrolPath == -1 || x8f2_curPatrolPathWaypoint == -1) {
    return {};
  }
  s8 tmpWpIdx = x8f2_curPatrolPathWaypoint;
  x8f2_curPatrolPathWaypoint = (x8f2_curPatrolPathWaypoint + 1) % x578_waypoints[x8f1_curPatrolPath].size();
  return TCastToConstPtr<CScriptWaypoint>(
             mgr.GetObjectById(x578_waypoints[x8f1_curPatrolPath][x8f2_curPatrolPathWaypoint]))
      ->GetTranslation();
}

zeus::CVector2f CThardus::GetSteeringVector(CStateManager& mgr) const {
  zeus::CVector2f ret;
  zeus::CVector3f pos = GetTranslation();
  for (const auto& repulsor : x664_repulsors) {
    TCastToConstPtr<CRepulsor> rep = mgr.GetObjectById(repulsor);
    const zeus::CVector2f repPos = rep->GetTranslation().toVec2f();
    const float dist = (pos - repPos).magSquared();
    if (dist < (rep->GetAffectRadius() * rep->GetAffectRadius())) {
      // std::sqrt(dist);
      x45c_steeringBehaviors.Flee2D(*this, repPos);
      break;
    }
  }

  if (ret != zeus::skZero2f) {
    ret = x45c_steeringBehaviors.Arrival2D(*this, x764_startTransform.origin.toVec2f()) * zeus::CVector2f(1.f) +
          ret * zeus::CVector2f(0.f);
  }
  return ret;
}

bool CThardus::sub801db5b4(CStateManager& mgr) const {
  return mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::Thermal ? !x93a_ || x7c4_ == 0 : true;
}

void CThardus::ApplyCameraShake(float magnitude, float sfxDistance, float duration, CStateManager& mgr,
                                const zeus::CVector3f& v1) {
  float bounceIntensity =
      std::max(0.f, -((v1 - mgr.GetPlayer().GetTranslation()).magnitude() * (magnitude / sfxDistance) - magnitude));
  if (mgr.GetCameraManager()->GetFirstPersonCamera()->GetUniqueId() == mgr.GetCameraManager()->GetCurrentCameraId()) {
    mgr.GetCameraManager()->AddCameraShaker(
        CCameraShakeData::BuildMissileCameraShake(duration, magnitude, sfxDistance, GetTranslation()), true);
  }

  if (x908_) {
    BouncePlayer(bounceIntensity, mgr);
  }
}

void CThardus::UpdateHealthInfo(CStateManager& mgr) {
  // TODO(phil): This isn't quite right, need to figure out why
  float hp = 0.f;
  for (size_t i = x648_currentRock; i < x610_destroyableRocks.size(); ++i) {
    float fVar1 = x648_currentRock == (x610_destroyableRocks.size() - 1) ? 2.f * x6a4_ : x6a4_;
    if (auto* rock = static_cast<CDestroyableRock*>(mgr.ObjectById(x610_destroyableRocks[i]))) {
      if (!rock->IsUsingPhazonModel()) {
        hp += fVar1;
      }
      hp += rock->GetHealthInfo(mgr)->GetHP();
    }
  }
  HealthInfo(mgr)->SetHP(hp);
}

void CThardus::BouncePlayer(float intensity, CStateManager& mgr) {
  if (intensity <= 0.f) {
    return;
  }
  zeus::CVector3f posDiff = GetTranslation() - mgr.GetPlayer().GetTranslation();
  CPlayer::ESurfaceRestraints restraints = mgr.GetPlayer().GetSurfaceRestraint();
  if (restraints != CPlayer::ESurfaceRestraints::Air && !mgr.GetPlayer().IsInWaterMovement()) {
    zeus::CVector3f baseImpulse = intensity * (40.f * zeus::skUp);
    zeus::CVector3f additionalImpulse;
    if (posDiff.magnitude() > 10.f) {
      zeus::CVector3f tmpVec = posDiff.toVec2f();
      if (tmpVec.canBeNormalized()) {
        additionalImpulse = intensity * (12.5f * tmpVec.normalized());
      }
    }
    mgr.GetPlayer().ApplyImpulseWR(mgr.GetPlayer().GetMass() * (baseImpulse + additionalImpulse), zeus::CAxisAngle());
    mgr.GetPlayer().SetMoveState(CPlayer::EPlayerMovementState::ApplyJump, mgr);
  }
}

void CThardus::sub801dbc40() {
  x7b8_ = FLT_EPSILON + x7bc_;
  x938_ = false;
}

zeus::CVector2f CThardus::GetTargetVector(float arg, CStateManager& mgr) {
  zeus::CVector2f ret;
  if (GetSearchPath() != nullptr) {
    if (GetSearchPath()->GetResult() == CPathFindSearch::EResult::Success) {
      CPatterned::PathFind(mgr, EStateMsg::Update, arg);
      ret = GetBodyController()->GetCommandMgr().GetMoveVector().toVec2f();
    } else {
      ret = x45c_steeringBehaviors.Arrival(*this, x7d8_, 0.f).toVec2f();
    }
  }

  if (x8d4_ || (!x8d4_ && x7f0_pathFindSearch.OnPath(GetTranslation()) != CPathFindSearch::EResult::Success)) {
    zeus::CVector2f vec = GetSteeringVector(mgr);
    if (vec != zeus::skZero2f) {
      return vec;
    }
  }

  if (ret == zeus::skZero2f) {
    return x45c_steeringBehaviors.Arrival(*this, x7d8_, 0.f).toVec2f();
  }

  return ret;
}
} // namespace metaforce::MP1
