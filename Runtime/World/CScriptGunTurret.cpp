#include "Runtime/World/CScriptGunTurret.hpp"

#include <array>

#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Character/CPASAnimParmData.hpp"
#include "Runtime/Collision/CCollisionActor.hpp"
#include "Runtime/Collision/CCollisionActorManager.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/Weapon/CEnergyProjectile.hpp"
#include "Runtime/Weapon/CGameProjectile.hpp"
#include "Runtime/World/CAiFuncMap.hpp"
#include "Runtime/World/CGameLight.hpp"
#include "Runtime/World/CPlayer.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {
namespace {
constexpr CMaterialList skGunMaterialList = {EMaterialTypes::Solid, EMaterialTypes::Character, EMaterialTypes::Orbit,
                                             EMaterialTypes::Target};
constexpr CMaterialList skTurretMaterialList = {EMaterialTypes::Character};

constexpr std::array<SBurst, 6> skBurst2InfoTemplate{{
    {3, {1, 2, -1, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {3, {7, 6, -1, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {4, {3, 5, -1, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {60, {16, 4, -1, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {30, {4, 4, -1, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
}};

constexpr std::array<SBurst, 6> skBurst3InfoTemplate{{
    {30, {4, 5, 4, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {30, {2, 3, 4, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {30, {3, 4, 5, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {5, {16, 1, 2, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {5, {8, 7, 6, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
}};

constexpr std::array<SBurst, 8> skBurst4InfoTemplate{{
    {5, {16, 1, 2, 3, 0, 0, 0, 0}, 0.150000, 0.050000},
    {5, {9, 8, 7, 6, 0, 0, 0, 0}, 0.150000, 0.050000},
    {15, {2, 3, 4, 5, 0, 0, 0, 0}, 0.150000, 0.050000},
    {15, {5, 4, 3, 2, 0, 0, 0, 0}, 0.150000, 0.050000},
    {15, {10, 11, 4, 13, 0, 0, 0, 0}, 0.150000, 0.050000},
    {15, {14, 13, 4, 11, 0, 0, 0, 0}, 0.150000, 0.050000},
    {30, {2, 4, 4, 6, 0, 0, 0, 0}, 0.150000, 0.050000},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
}};

constexpr std::array<SBurst, 6> skOOVBurst2InfoTemplate{{
    {20, {16, 15, -1, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {20, {8, 9, -1, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {20, {13, 11, -1, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {20, {2, 6, -1, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {20, {3, 4, -1, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
}};

constexpr std::array<SBurst, 6> skOOVBurst3InfoTemplate{{
    {10, {14, 4, 10, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {10, {15, 13, 4, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {10, {9, 11, 4, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {35, {15, 13, 11, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {35, {9, 11, 13, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
}};

constexpr std::array<SBurst, 7> skOOVBurst4InfoTemplate{{
    {10, {14, 13, 4, 11, 0, 0, 0, 0}, 0.150000, 0.050000},
    {30, {1, 15, 13, 11, 0, 0, 0, 0}, 0.150000, 0.050000},
    {20, {16, 15, 14, 13, 0, 0, 0, 0}, 0.150000, 0.050000},
    {10, {8, 9, 11, 4, 0, 0, 0, 0}, 0.150000, 0.050000},
    {10, {1, 15, 13, 4, 0, 0, 0, 0}, 0.150000, 0.050000},
    {20, {8, 9, 10, 11, 0, 0, 0, 0}, 0.150000, 0.050000},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
}};

constexpr std::array<const SBurst*, 7> skBursts{
    skBurst2InfoTemplate.data(),
    skBurst3InfoTemplate.data(),
    skBurst4InfoTemplate.data(),
    skOOVBurst2InfoTemplate.data(),
    skOOVBurst3InfoTemplate.data(),
    skOOVBurst4InfoTemplate.data(),
    nullptr,
};

constexpr std::array StateNames{
    "Destroyed", "Deactive", "DeactiveFromReady", "Deactivating", "DeactivatingFromReady", "Inactive", "Ready",
    "PanningA",  "PanningB", "Targeting",         "Firing",       "ExitTargeting",         "Frenzy",
};

constexpr std::array<u32, 13> skStateToLocoTypeLookup{
    5, 7, 9, 0, 1, 0, 1, 2, 3, 1, 1, 1, 1,
};
} // Anonymous namespace

CScriptGunTurretData::CScriptGunTurretData(CInputStream& in, s32 propCount)
: x0_intoDeactivateDelay(in.readFloatBig())
, x4_intoActivateDelay(in.readFloatBig())
, x8_reloadTime(in.readFloatBig())
, xc_reloadTimeVariance(in.readFloatBig())
, x10_panStartTime(in.readFloatBig())
, x14_panHoldTime(in.readFloatBig())
, x1c_leftMaxAngle(zeus::degToRad(in.readFloatBig()))
, x20_rightMaxAngle(zeus::degToRad(in.readFloatBig()))
, x24_downMaxAngle(zeus::degToRad(in.readFloatBig()))
, x28_turnSpeed(zeus::degToRad(in.readFloatBig()))
, x2c_detectionRange(in.readFloatBig())
, x30_detectionZRange(in.readFloatBig())
, x34_freezeDuration(in.readFloatBig())
, x38_freezeVariance(in.readFloatBig())
, x3c_freezeTimeout(propCount >= 48 ? in.readBool() : false)
, x40_projectileRes(in)
, x44_projectileDamage(in)
, x60_idleLightRes(in.readUint32Big())
, x64_deactivateLightRes(in.readUint32Big())
, x68_targettingLightRes(in.readUint32Big())
, x6c_frozenEffectRes(in.readUint32Big())
, x70_chargingEffectRes(in.readUint32Big())
, x74_panningEffectRes(in.readUint32Big())
, x78_visorEffectRes(propCount >= 44 ? in.readUint32Big() : -1)
, x7c_trackingSoundId(CSfxManager::TranslateSFXID(u16(in.readUint32Big())))
, x7e_lockOnSoundId(CSfxManager::TranslateSFXID(u16(in.readUint32Big())))
, x80_unfreezeSoundId(CSfxManager::TranslateSFXID(u16(in.readUint32Big())))
, x82_stopClankSoundId(CSfxManager::TranslateSFXID(u16(in.readUint32Big())))
, x84_chargingSoundId(CSfxManager::TranslateSFXID(u16(in.readUint32Big())))
, x86_visorSoundId(propCount >= 45 ? CSfxManager::TranslateSFXID(u16(in.readUint32Big())) : u16(0xFFFF))
, x88_extensionModelResId(in.readUint32Big())
, x8c_extensionDropDownDist(in.readFloatBig())
, x90_numInitialShots(in.readUint32Big())
, x94_initialShotTableIndex(in.readUint32Big())
, x98_numSubsequentShots(in.readUint32Big())
, x9c_frenzyDuration(propCount >= 47 ? in.readFloatBig() : 3.f)
, xa0_scriptedStartOnly(propCount >= 46 ? in.readBool() : false) {}

CScriptGunTurret::CScriptGunTurret(TUniqueId uid, std::string_view name, ETurretComponent comp, const CEntityInfo& info,
                                   const zeus::CTransform& xf, CModelData&& mData, const zeus::CAABox& aabb,
                                   const CHealthInfo& hInfo, const CDamageVulnerability& dVuln,
                                   const CActorParameters& aParms, const CScriptGunTurretData& turretData)
: CPhysicsActor(uid, true, name, info, xf, std::move(mData),
                comp == ETurretComponent::Base ? skTurretMaterialList : skGunMaterialList, aabb, SMoverData(1000.f),
                aParms, 0.3f, 0.1f)
, x258_type(comp)
, x264_healthInfo(hInfo)
, x26c_damageVuln(dVuln)
, x2d4_data(turretData)
, x37c_projectileInfo(turretData.GetProjectileRes(), turretData.GetProjectileDamage())
, x3a4_burstFire(skBursts.data(), 1)
, x410_idleLightDesc(g_SimplePool->GetObj({SBIG('PART'), turretData.GetIdleLightRes()}))
, x41c_deactivateLightDesc(g_SimplePool->GetObj({SBIG('PART'), turretData.GetDeactivateLightRes()}))
, x428_targettingLightDesc(g_SimplePool->GetObj({SBIG('PART'), turretData.GetTargettingLightRes()}))
, x434_frozenEffectDesc(g_SimplePool->GetObj({SBIG('PART'), turretData.GetFrozenEffectRes()}))
, x440_chargingEffectDesc(g_SimplePool->GetObj({SBIG('PART'), turretData.GetChargingEffectRes()}))
, x44c_panningEffectDesc(g_SimplePool->GetObj({SBIG('PART'), turretData.GetPanningEffectRes()})) {
  if (turretData.GetVisorEffectRes().IsValid())
    x458_visorEffectDesc = g_SimplePool->GetObj({SBIG('PART'), turretData.GetVisorEffectRes()});
  x468_idleLight = std::make_unique<CElementGen>(x410_idleLightDesc);
  x470_deactivateLight = std::make_unique<CElementGen>(x41c_deactivateLightDesc);
  x478_targettingLight = std::make_unique<CElementGen>(x428_targettingLightDesc);
  x480_frozenEffect = std::make_unique<CElementGen>(x434_frozenEffectDesc);
  x488_chargingEffect = std::make_unique<CElementGen>(x440_chargingEffectDesc);
  x490_panningEffect = std::make_unique<CElementGen>(x44c_panningEffectDesc);
  x4fc_extensionOffset = xf.origin;
  x514_lastFrontVector = xf.frontVector();
  x544_originalFrontVec = xf.frontVector();
  x550_originalRightVec = xf.rightVector();

  if (comp == ETurretComponent::Base && HasModelData() && GetModelData()->HasAnimData())
    GetModelData()->EnableLooping(true);

  x37c_projectileInfo.Token().Lock();
}

CScriptGunTurret::~CScriptGunTurret() = default;

void CScriptGunTurret::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptGunTurret::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, uid, mgr);

  switch (msg) {
  case EScriptObjectMessage::Activate:
    if (x49c_collisionManager)
      x49c_collisionManager->SetActive(mgr, true);
    break;
  case EScriptObjectMessage::Deactivate:
    if (x49c_collisionManager)
      x49c_collisionManager->SetActive(mgr, false);
    break;
  case EScriptObjectMessage::Registered:
    if (x258_type == ETurretComponent::Gun) {
      if (x478_targettingLight->SystemHasLight()) {
        x498_lightId = mgr.AllocateUniqueId();
        mgr.AddObject(new CGameLight(x498_lightId, GetAreaIdAlways(), GetActive(),
                                     std::string("ParticleLight_").append(GetName()), GetTransform(), GetUniqueId(),
                                     x478_targettingLight->GetLight(), 0, 1, 0.f));
      }
      SetupCollisionManager(mgr);
    } else if (x258_type == ETurretComponent::Base) {
      zeus::CVector3f scale = GetModelData()->GetScale();
      if (x2d4_data.GetExtensionModelResId().IsValid()) {
        CModelData mData(CStaticRes(x2d4_data.GetExtensionModelResId(), scale));
        x4a4_extensionModel.emplace(std::move(mData));
        x4f4_extensionRange = x4a4_extensionModel->GetBounds().max.z() - x4a4_extensionModel->GetBounds().min.z();
      }
      SetTurretState(ETurretState::Inactive, mgr);
    }
    break;
  case EScriptObjectMessage::Deleted: {
    if (x258_type == ETurretComponent::Gun) {
      if (x498_lightId != kInvalidUniqueId)
        mgr.FreeScriptObject(x498_lightId);
    }
    if (x50c_targetingEmitter)
      CSfxManager::RemoveEmitter(x50c_targetingEmitter);

    if (x49c_collisionManager)
      x49c_collisionManager->Destroy(mgr);
    break;
  }
  case EScriptObjectMessage::Start:
    if (x258_type == ETurretComponent::Base && x520_state == ETurretState::Inactive)
      x560_29_scriptedStart = true;
    break;
  case EScriptObjectMessage::Stop:
    if (x258_type == ETurretComponent::Base && x520_state != ETurretState::Deactive &&
        x520_state != ETurretState::DeactiveFromReady && x520_state != ETurretState::Deactivating)
      SetTurretState((x560_28_hasBeenActivated ?
      ETurretState::DeactivatingFromReady : ETurretState::Deactivating), mgr);
    break;
  case EScriptObjectMessage::Action: {
    if (x258_type == ETurretComponent::Gun)
      LaunchProjectile(mgr);
    else if (x258_type == ETurretComponent::Base)
      PlayAdditiveFlinchAnimation(mgr);
    break;
  }
  case EScriptObjectMessage::SetToMax: {
    x560_25_frozen = false;
    SetMuted(false);
    break;
  }
  case EScriptObjectMessage::SetToZero: {
    x560_25_frozen = true;
    SetMuted(true);
    break;
  }
  case EScriptObjectMessage::InitializedInArea: {
    if (x258_type == ETurretComponent::Base) {
      for (const SConnection& conn : x20_conns) {
        if (conn.x0_state != EScriptObjectState::Play || conn.x4_msg != EScriptObjectMessage::Activate)
          continue;

        if (TCastToConstPtr<CScriptGunTurret> gun = mgr.GetObjectById(mgr.GetIdForScript(conn.x8_objId))) {
          x25c_gunId = mgr.GetIdForScript(conn.x8_objId);
          x260_lastGunHP = gun->GetHealthInfo(mgr)->GetHP();
          return;
        }
      }
    }
    break;
  }
  case EScriptObjectMessage::Damage: {
    if (x258_type == ETurretComponent::Gun && GetHealthInfo(mgr)->GetHP() > 0.f) {
      if (TCastToConstPtr<CGameProjectile> proj = mgr.GetObjectById(uid)) {
        if ((proj->GetAttribField() & EProjectileAttrib::Wave) == EProjectileAttrib::Wave) {
          x520_state = ETurretState::Frenzy;
          RemoveMaterial(EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
          mgr.GetPlayer().SetOrbitRequestForTarget(GetUniqueId(), CPlayer::EPlayerOrbitRequest::ActivateOrbitSource,
                                                   mgr);
          x53c_freezeRemTime = 0.f;
        }
      }
    }
    break;
  }
  default:
    break;
  }
}

void CScriptGunTurret::Think(float dt, CStateManager& mgr) {
  if (!GetActive())
    return;

  if (x258_type == ETurretComponent::Base) {
    if (!x560_25_frozen) {
      ProcessGunStateMachine(dt, mgr);
      UpdateTurretAnimation();
      UpdateGunOrientation(dt, mgr);
      zeus::CVector3f vec = UpdateExtensionModelState(dt);
      SAdvancementDeltas advancementDeltas = UpdateAnimation(dt, mgr, true);
      SetTranslation(vec + advancementDeltas.x0_posDelta + GetTranslation());
      RotateToOR(advancementDeltas.xc_rotDelta, dt);
    } else
      Stop();

    UpdateTargettingSound(dt);
  } else if (x258_type == ETurretComponent::Gun) {
    UpdateGunParticles(dt, mgr);
    SAdvancementDeltas deltas = UpdateAnimation(dt, mgr, true);
    MoveToOR(deltas.x0_posDelta, dt);
    RotateToOR(deltas.xc_rotDelta, dt);
    UpdateGunCollisionManager(dt, mgr);
    UpdateFrozenState(dt, mgr);
  }
  UpdateHealthInfo(mgr);
}

void CScriptGunTurret::Touch(CActor& act, CStateManager& mgr) {
  if (x258_type != ETurretComponent::Gun)
    return;
  if (TCastToPtr<CGameProjectile> proj = act) {
    const CPlayer& player = mgr.GetPlayer();
    if (proj->GetOwnerId() == player.GetUniqueId()) {
      const CDamageVulnerability* dVuln = GetDamageVulnerability();
      if (!x560_24_dead && x520_state != ETurretState::Frenzy &&
          (proj->GetAttribField() & EProjectileAttrib::Ice) == EProjectileAttrib::Ice &&
          dVuln->WeaponHits(CWeaponMode::Ice(), false)) {
        x560_25_frozen = true;
        SendScriptMsgs(EScriptObjectState::Zero, mgr, EScriptObjectMessage::None);
        x53c_freezeRemTime = mgr.GetActiveRandom()->Float() *
          x2d4_data.GetFreezeVariance() + x2d4_data.GetFreezeDuration();
        SetMuted(true);
      }
      SendScriptMsgs(EScriptObjectState::Damage, mgr, EScriptObjectMessage::None);
    }
  }
}

std::optional<zeus::CAABox> CScriptGunTurret::GetTouchBounds() const {
  if (GetActive() && GetMaterialList().HasMaterial(EMaterialTypes::Solid)) {
    return GetBoundingBox();
  }
  return std::nullopt;
}

zeus::CVector3f CScriptGunTurret::GetOrbitPosition(const CStateManager& mgr) const { return GetAimPosition(mgr, 0.f); }

zeus::CVector3f CScriptGunTurret::GetAimPosition(const CStateManager&, float) const {
  if (x258_type == ETurretComponent::Base)
    return GetTranslation() + x34_transform.rotate(GetLocatorTransform("Gun_SDK"sv).origin);

  return GetTranslation();
}

void CScriptGunTurret::SetupCollisionManager(CStateManager& mgr) {
  std::vector<CJointCollisionDescription> jointDescs;
  jointDescs.reserve(2);
  const CAnimData* animData = GetModelData()->GetAnimationData();
  x508_gunSDKSeg = animData->GetLocatorSegId("Gun_SDK"sv);
  CSegId blastLCTR = animData->GetLocatorSegId("Blast_LCTR"sv);
  jointDescs.push_back(CJointCollisionDescription::SphereSubdivideCollision(
      x508_gunSDKSeg, blastLCTR, 0.6f, 1.f, CJointCollisionDescription::EOrientationType::One, "Gun_SDK"sv, 1000.f));
  jointDescs.push_back(CJointCollisionDescription::SphereCollision(blastLCTR, 0.3f, "Blast_LCTR"sv, 1000.f));

  x49c_collisionManager = std::make_unique<CCollisionActorManager>(mgr, GetUniqueId(), GetAreaIdAlways(), jointDescs, true);
  x49c_collisionManager->SetActive(mgr, GetActive());

  for (int i = 0; i < x49c_collisionManager->GetNumCollisionActors(); ++i) {
    auto& desc = x49c_collisionManager->GetCollisionDescFromIndex(i);
    if (TCastToPtr<CCollisionActor> cAct = mgr.ObjectById(desc.GetCollisionActorId())) {
      cAct->AddMaterial(EMaterialTypes::ProjectilePassthrough, mgr);
      cAct->SetMaterialFilter(CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Player},
        {EMaterialTypes::Character, EMaterialTypes::NoStaticCollision, EMaterialTypes::NoPlatformCollision}));
      if (desc.GetName().find("Blast_LCTR"sv) != std::string_view::npos)
        x4a0_collisionActor = desc.GetCollisionActorId();
    }
  }
}

void CScriptGunTurret::SetTurretState(ETurretState state, CStateManager& mgr) {
  if (state < ETurretState::Destroyed || state > ETurretState::Frenzy) {
    return;
  }

  if (x520_state != ETurretState::Invalid) {
    ProcessCurrentState(EStateMsg::Deactivate, mgr, 0.f);
  }

  if (state != ETurretState::Invalid && x520_state != state) {
#ifndef NDEBUG
    fmt::print(FMT_STRING("{} {} {} - {}\n"), GetUniqueId(), GetEditorId(), GetName(), StateNames[size_t(state)]);
#endif
  }

  x520_state = state;
  x524_curStateTime = 0.f;
  ProcessCurrentState(EStateMsg::Activate, mgr, 0.f);
}

void CScriptGunTurret::LaunchProjectile(CStateManager& mgr) {
  if (x37c_projectileInfo.Token().IsLoaded() && mgr.CanCreateProjectile(GetUniqueId(), EWeaponType::AI, 8)) {
    zeus::CTransform xf = GetLocatorTransform("Blast_LCTR"sv);
    zeus::CVector3f projPt = GetTranslation() + GetTransform().rotate(xf.origin);
    zeus::CVector3f lookPt = x404_targetPosition;
    zeus::CVector3f aimDelta = x404_targetPosition - projPt;
    if (zeus::CVector3f::getAngleDiff(GetTransform().frontVector(), aimDelta) > zeus::degToRad(20.f)) {
      if (aimDelta.canBeNormalized()) {
        zeus::CVector3f lookDir =
          zeus::CVector3f::slerp(GetTransform().frontVector(), aimDelta.normalized(), zeus::degToRad(20.f));
        lookPt = lookDir * aimDelta.magnitude() + projPt;
      } else {
        lookPt = projPt + GetTransform().frontVector();
      }
    } else if (!aimDelta.canBeNormalized()) {
      lookPt = projPt + GetTransform().frontVector();
    }
    zeus::CTransform useXf = zeus::lookAt(projPt, lookPt);
    CEnergyProjectile* proj =
    new CEnergyProjectile(true, x37c_projectileInfo.Token(), EWeaponType::AI, useXf, EMaterialTypes::Character,
                          x37c_projectileInfo.GetDamage(), mgr.AllocateUniqueId(), GetAreaIdAlways(), GetUniqueId(),
                          kInvalidUniqueId, EProjectileAttrib::None, false, zeus::skOne3f,
                          x458_visorEffectDesc, x2d4_data.GetVisorSoundId(), false);
    mgr.AddObject(proj);
    auto pair =
    x64_modelData->GetAnimationData()->GetCharacterInfo().GetPASDatabase().FindBestAnimation(
    CPASAnimParmData(18, CPASAnimParm::FromEnum(1), CPASAnimParm::FromReal32(90.f),
      CPASAnimParm::FromEnum(skStateToLocoTypeLookup[size_t(x520_state)])), -1);
    if (pair.first > 0.f) {
      x64_modelData->EnableLooping(false);
      x64_modelData->GetAnimationData()->SetAnimation(CAnimPlaybackParms(pair.second, -1, 1.f, true), false);
    }
  }
}

void CScriptGunTurret::PlayAdditiveFlinchAnimation(CStateManager& mgr) {
  auto pair = GetModelData()->GetAnimationData()->GetCharacterInfo().GetPASDatabase().FindBestAnimation(
      CPASAnimParmData(23), *mgr.GetActiveRandom(), -1);
  if (pair.first > 0.f)
    GetModelData()->GetAnimationData()->AddAdditiveAnimation(pair.second, 1.f, false, true);
}

void CScriptGunTurret::AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) {
  CActor::AddToRenderer(frustum, mgr);

  if (x258_type != ETurretComponent::Gun) {
    return;
  }

  if (!x560_25_frozen) {
    switch (x520_state) {
    case ETurretState::Deactive:
    case ETurretState::DeactiveFromReady:
    case ETurretState::Deactivating:
    case ETurretState::DeactivatingFromReady:
      g_Renderer->AddParticleGen(*x470_deactivateLight);
      break;
    case ETurretState::Inactive:
      g_Renderer->AddParticleGen(*x468_idleLight);
      break;
    case ETurretState::PanningA:
    case ETurretState::PanningB:
      g_Renderer->AddParticleGen(*x490_panningEffect);
      break;
    case ETurretState::Ready:
    case ETurretState::Targeting:
    case ETurretState::Firing:
    case ETurretState::ExitTargeting:
    case ETurretState::Frenzy:
      g_Renderer->AddParticleGen(*x478_targettingLight);
      if (x520_state == ETurretState::Firing || x520_state == ETurretState::Frenzy) {
        g_Renderer->AddParticleGen(*x488_chargingEffect);
      }
      break;
    default:
      break;
    }
  } else {
    g_Renderer->AddParticleGen(*x480_frozenEffect);
  }
}

void CScriptGunTurret::Render(CStateManager& mgr) {
  CPhysicsActor::Render(mgr);

  if (x258_type == ETurretComponent::Gun) {
    if (!x560_25_frozen) {
      switch (x520_state) {
      case ETurretState::Deactive:
      case ETurretState::DeactiveFromReady:
      case ETurretState::Deactivating:
      case ETurretState::DeactivatingFromReady:
        x470_deactivateLight->Render(x90_actorLights.get());
        break;
      case ETurretState::Inactive:
        x468_idleLight->Render(x90_actorLights.get());
        break;
      case ETurretState::PanningA:
      case ETurretState::PanningB:
        x490_panningEffect->Render(x90_actorLights.get());
        break;
      case ETurretState::Ready:
      case ETurretState::Targeting:
      case ETurretState::Firing:
      case ETurretState::ExitTargeting:
      case ETurretState::Frenzy:
        x478_targettingLight->Render(x90_actorLights.get());
        if (x520_state == ETurretState::Firing)
          x488_chargingEffect->Render(x90_actorLights.get());
        break;
      default:
        break;
      }
    } else {
      x480_frozenEffect->Render(x90_actorLights.get());
    }
  } else if (x258_type == ETurretComponent::Base) {
    if (x4a4_extensionModel && x4f8_extensionT > 0.f) {
      zeus::CTransform xf = GetTransform();
      xf.origin = x4fc_extensionOffset + (x4f4_extensionRange * 0.5f * zeus::skDown);
      CModelFlags flags;
      flags.x2_flags = 3;
      flags.x1_matSetIdx = 0;
      flags.x4_color = zeus::skWhite;
      x4a4_extensionModel->Render(mgr, xf, x90_actorLights.get(), flags);
    }
  }
}

void CScriptGunTurret::UpdateGunCollisionManager(float dt, CStateManager& mgr) {
  if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(x4a0_collisionActor))
    colAct->SetActive(mgr.GetPlayer().GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed);

  x49c_collisionManager->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);
}

void CScriptGunTurret::UpdateFrozenState(float dt, CStateManager& mgr) {
  if (x560_25_frozen) {
    if (x53c_freezeRemTime <= 0.f) {
      x560_25_frozen = false;
      SendScriptMsgs(EScriptObjectState::UnFrozen, mgr, EScriptObjectMessage::None);
      CSfxManager::AddEmitter(x2d4_data.GetUnFreezeSoundId(), GetTranslation(), zeus::skUp, false, false,
                              0x7f, GetAreaIdAlways());
      SetMuted(false);
    } else if (x2d4_data.GetFreezeTimeout()) {
      x53c_freezeRemTime -= dt;
    }
  } else {
    x53c_freezeRemTime = 0.f;
  }
}

void CScriptGunTurret::UpdateGunParticles(float dt, CStateManager& mgr) {
  CGameLight* light = nullptr;
  if (x498_lightId != kInvalidUniqueId)
    light = TCastToPtr<CGameLight>(mgr.ObjectById(x498_lightId));

  if (!x560_25_frozen) {
    zeus::CTransform lightXf = GetLocatorTransform("light_LCTR"sv);
    zeus::CVector3f pos = x34_transform.rotate(lightXf.origin);
    pos += GetTranslation();
    if (light)
      light->SetActive(true);

    switch (x520_state) {
    case ETurretState::Deactive:
    case ETurretState::DeactiveFromReady:
    case ETurretState::Deactivating:
    case ETurretState::DeactivatingFromReady:
      x468_idleLight->SetParticleEmission(false);
      x470_deactivateLight->SetParticleEmission(true);
      x478_targettingLight->SetParticleEmission(false);
      x480_frozenEffect->SetParticleEmission(false);
      x488_chargingEffect->SetParticleEmission(false);
      x490_panningEffect->SetParticleEmission(false);
      x470_deactivateLight->SetOrientation(GetTransform().getRotation());
      x470_deactivateLight->SetGlobalTranslation(pos);
      x470_deactivateLight->SetGlobalScale(GetModelData()->GetScale());
      x470_deactivateLight->Update(dt);
      if (light) {
        if (x470_deactivateLight->SystemHasLight())
          light->SetLight(x470_deactivateLight->GetLight());
        else
          light->SetActive(false);
      }
      break;
    case ETurretState::Inactive:
      x468_idleLight->SetParticleEmission(true);
      x470_deactivateLight->SetParticleEmission(false);
      x478_targettingLight->SetParticleEmission(false);
      x480_frozenEffect->SetParticleEmission(false);
      x488_chargingEffect->SetParticleEmission(false);
      x490_panningEffect->SetParticleEmission(false);
      x468_idleLight->SetOrientation(GetTransform().getRotation());
      x468_idleLight->SetGlobalTranslation(pos);
      x468_idleLight->SetGlobalScale(GetModelData()->GetScale());
      x468_idleLight->Update(dt);
      if (light)
        light->SetActive(false);
      break;
    case ETurretState::PanningA:
    case ETurretState::PanningB:
      x468_idleLight->SetParticleEmission(false);
      x470_deactivateLight->SetParticleEmission(false);
      x478_targettingLight->SetParticleEmission(false);
      x480_frozenEffect->SetParticleEmission(false);
      x488_chargingEffect->SetParticleEmission(false);
      x490_panningEffect->SetParticleEmission(true);
      x490_panningEffect->SetOrientation(GetTransform().getRotation());
      x490_panningEffect->SetGlobalTranslation(GetTranslation());
      x490_panningEffect->SetGlobalScale(GetModelData()->GetScale());
      x490_panningEffect->Update(dt);
      if (light)
        light->SetActive(false);
      break;
    case ETurretState::Targeting:
    case ETurretState::Firing:
    case ETurretState::ExitTargeting:
    case ETurretState::Frenzy: {
      bool doEmission = x520_state == ETurretState::Firing || x520_state == ETurretState::Frenzy;
      x468_idleLight->SetParticleEmission(false);
      x470_deactivateLight->SetParticleEmission(false);
      x478_targettingLight->SetParticleEmission(true);
      x480_frozenEffect->SetParticleEmission(false);
      x488_chargingEffect->SetParticleEmission(doEmission);
      x478_targettingLight->SetOrientation(GetTransform().getRotation());
      x478_targettingLight->SetGlobalTranslation(pos);
      x478_targettingLight->SetGlobalScale(GetModelData()->GetScale());
      x478_targettingLight->Update(dt);
      if (x478_targettingLight->SystemHasLight())
        light->SetLight(x478_targettingLight->GetLight());
      else
        light->SetActive(false);

      if (doEmission) {
        zeus::CTransform blastXf = GetLocatorTransform("Blast_LCTR"sv);
        zeus::CVector3f blastPos = GetTransform().rotate(blastXf.origin);
        blastPos += GetTranslation();
        x488_chargingEffect->SetOrientation(GetTransform().getRotation());
        x488_chargingEffect->SetGlobalTranslation(blastPos);
        x488_chargingEffect->SetGlobalScale(GetModelData()->GetScale());
        x488_chargingEffect->Update(dt);
      }
      break;
    }
    default:
      x468_idleLight->SetParticleEmission(false);
      x470_deactivateLight->SetParticleEmission(false);
      x478_targettingLight->SetParticleEmission(false);
      x480_frozenEffect->SetParticleEmission(false);
      x488_chargingEffect->SetParticleEmission(false);
      x490_panningEffect->SetParticleEmission(false);
      x490_panningEffect->Update(dt);
      if (light)
        light->SetActive(false);
      break;
    }
  } else {
    x468_idleLight->SetParticleEmission(false);
    x470_deactivateLight->SetParticleEmission(false);
    x478_targettingLight->SetParticleEmission(false);
    x480_frozenEffect->SetParticleEmission(true);
    x488_chargingEffect->SetParticleEmission(false);
    x490_panningEffect->SetParticleEmission(false);
    x480_frozenEffect->SetOrientation(GetTransform().getRotation());
    x480_frozenEffect->SetGlobalTranslation(GetTranslation());
    x480_frozenEffect->SetGlobalScale(GetModelData()->GetScale());
    x480_frozenEffect->Update(dt);
    if (light)
      light->SetActive(false);
  }
}

void CScriptGunTurret::ProcessGunStateMachine(float dt, CStateManager& mgr) {
  ProcessCurrentState(EStateMsg::Update, mgr, dt);
  x524_curStateTime += dt;
  PlayAdditiveChargingAnimation(mgr);
  if (x25c_gunId != kInvalidUniqueId) {
    if (TCastToPtr<CScriptGunTurret> gunTurret = mgr.ObjectById(x25c_gunId)) {
      if (gunTurret->x520_state != ETurretState::Frenzy)
        gunTurret->x520_state = x520_state;
      else if (x520_state != ETurretState::Frenzy) {
        SetTurretState(ETurretState::Frenzy, mgr);
        gunTurret->RemoveMaterial(EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
        mgr.GetPlayer().SetOrbitRequestForTarget(GetUniqueId(), CPlayer::EPlayerOrbitRequest::ActivateOrbitSource, mgr);
      }
    }
  }
}

void CScriptGunTurret::UpdateTurretAnimation() {
  if (!HasModelData() || !GetModelData()->HasAnimData()) {
    return;
  }

  if (x520_state > ETurretState::Frenzy) {
    return;
  }

  const auto parmData = CPASAnimParmData(5, CPASAnimParm::FromEnum(0),
                                         CPASAnimParm::FromEnum(skStateToLocoTypeLookup[size_t(x520_state)]));
  const auto pair =
      GetModelData()->GetAnimationData()->GetCharacterInfo().GetPASDatabase().FindBestAnimation(parmData, -1);

  if (pair.first > 0.f && pair.second != x540_turretAnim) {
    GetModelData()->GetAnimationData()->SetAnimation(CAnimPlaybackParms(pair.second, -1, 1.f, true), false);
    GetModelData()->GetAnimationData()->EnableLooping(true);
    x540_turretAnim = pair.second;
  }
}

void CScriptGunTurret::ProcessCurrentState(EStateMsg msg, CStateManager& mgr, float dt) {
  switch (x520_state) {
  case ETurretState::Deactivating:
  case ETurretState::DeactivatingFromReady:
    ProcessDeactivatingState(msg, mgr);
    break;
  case ETurretState::Inactive:
    ProcessInactiveState(msg, mgr, dt);
    break;
  case ETurretState::Ready:
    ProcessReadyState(msg, mgr, dt);
    break;
  case ETurretState::PanningA:
  case ETurretState::PanningB:
    ProcessPanningState(msg, mgr, dt);
    break;
  case ETurretState::Targeting:
  case ETurretState::Firing:
    ProcessTargettingState(msg, mgr, dt);
    break;
  case ETurretState::ExitTargeting:
    ProcessExitTargettingState(msg, mgr);
    break;
  case ETurretState::Frenzy:
    ProcessFrenzyState(msg, mgr, dt);
    break;
  default:
    break;
  }
}

void CScriptGunTurret::ProcessDeactivatingState(EStateMsg msg, CStateManager& mgr) {
  if (msg == EStateMsg::Update && x524_curStateTime >= x2d4_data.GetIntoDeactivateDelay())
    SetTurretState(x560_28_hasBeenActivated ? ETurretState::DeactiveFromReady : ETurretState::Deactive, mgr);
}

void CScriptGunTurret::ProcessInactiveState(EStateMsg msg, CStateManager& mgr, float dt) {
  if (msg == EStateMsg::Activate) {
    x528_curInactiveTime = 0.f;
    x560_27_burstSet = false;
    if (TCastToPtr<CScriptGunTurret> gunTurret = mgr.ObjectById(x25c_gunId))
      x260_lastGunHP = gunTurret->HealthInfo(mgr)->GetHP();
  } else if (msg == EStateMsg::Update) {
    bool forceActivate = false;
    if (x25c_gunId != kInvalidUniqueId)
      if (TCastToPtr<CScriptGunTurret> gun = mgr.ObjectById(x25c_gunId))
        forceActivate = gun->HealthInfo(mgr)->GetHP() < x260_lastGunHP;
    if (x2d4_data.GetScriptedStartOnly() ? (forceActivate || x560_29_scriptedStart) :
        (forceActivate || x560_29_scriptedStart || InDetectionRange(mgr))) {
      x528_curInactiveTime += dt;
      if (forceActivate || x528_curInactiveTime >= x2d4_data.GetIntoActivateDelay())
        SetTurretState(ETurretState::Ready, mgr);
    } else {
      x468_idleLight->SetParticleEmission(true);
    }
  } else if (msg == EStateMsg::Deactivate) {
    x560_28_hasBeenActivated = true;
    x468_idleLight->SetParticleEmission(false);

    if (TCastToPtr<CScriptGunTurret> gunTurret = mgr.ObjectById(x25c_gunId))
      x260_lastGunHP = gunTurret->GetHealthInfo(mgr)->GetHP();
  }
}

void CScriptGunTurret::ProcessReadyState(EStateMsg msg, CStateManager& mgr, float dt) {
  if (msg == EStateMsg::Activate) {
    x52c_curActiveTime = 0.f;
  } else if (msg == EStateMsg::Update) {
    x52c_curActiveTime += dt;
    if (x52c_curActiveTime < x2d4_data.GetPanStartTime())
      return;

    if (IsPlayerInFiringRange(mgr) && InDetectionRange(mgr)) {
      SetTurretState(ETurretState::Targeting, mgr);
      CSfxManager::AddEmitter(x2d4_data.GetLockOnSoundId(), GetTranslation(), zeus::skUp, false, false, 0x7f,
                              GetAreaIdAlways());
    } else {
      SetTurretState(ETurretState::PanningA, mgr);
      x530_curPanTime = 0.f;
    }
  }
}

void CScriptGunTurret::ProcessPanningState(EStateMsg msg, CStateManager& mgr, float dt) {
  if (msg == EStateMsg::Activate) {
    x52c_curActiveTime = 0.f;
  } else if (msg == EStateMsg::Update) {
    if (IsPlayerInFiringRange(mgr) && InDetectionRange(mgr)) {
      SetTurretState(ETurretState::Targeting, mgr);
      CSfxManager::AddEmitter(x2d4_data.GetLockOnSoundId(), GetTranslation(), zeus::skUp, false, false, 0x7f,
                              GetAreaIdAlways());
    } else {
      x52c_curActiveTime += dt;
      x530_curPanTime += dt;
      if (x530_curPanTime >= x2d4_data.GetTotalPanSearchTime() &&
          !x4a4_extensionModel && !x2d4_data.GetScriptedStartOnly())
        SetTurretState(ETurretState::Inactive, mgr);
      else if (x52c_curActiveTime >= x2d4_data.GetPanHoldTime())
        SetTurretState(x520_state != ETurretState::PanningA ? ETurretState::PanningA : ETurretState::PanningB, mgr);
    }
  }
}

void CScriptGunTurret::ProcessTargettingState(EStateMsg msg, CStateManager& mgr, float dt) {
  if (msg == EStateMsg::Activate) {
    x52c_curActiveTime = 0.f;
  } else if (msg == EStateMsg::Update) {
    if (x560_26_firedWithSetBurst || InDetectionRange(mgr)) {
      UpdateTargettingMode(dt, mgr);
      if (x25c_gunId != kInvalidUniqueId) {
        if (TCastToPtr<CScriptGunTurret> gun = mgr.ObjectById(x25c_gunId)) {
          zeus::CVector3f vec = x404_targetPosition;
          if (IsPlayerInFiringRange(mgr)) {
            zeus::CTransform blastXf = gun->GetLocatorTransform("Blast_LCTR"sv);
            zeus::CVector3f rotatedBlastVec = GetTransform().rotate(blastXf.origin) + GetTranslation();
            x404_targetPosition = mgr.GetPlayer().GetAimPosition(mgr, 0.f);
            vec = x37c_projectileInfo.PredictInterceptPos(rotatedBlastVec, mgr.GetPlayer().GetAimPosition(mgr, dt),
                                                          mgr.GetPlayer(), false, dt);
          }

          zeus::CVector3f compensated =
              x3a4_burstFire.GetDistanceCompensatedError(
                (x404_targetPosition - gun->GetTranslation()).magnitude(), 20.f);

          compensated = gun->GetTransform().rotate(compensated);

          gun->x404_targetPosition = x404_targetPosition + (vec - x404_targetPosition) + compensated;
        }
      }

      zeus::CVector3f diffVec = x404_targetPosition - GetTranslation();
      diffVec.z() = 0.f;
      if (diffVec.canBeNormalized()) {
        zeus::CVector3f normDiff = diffVec.normalized();
        float angDif = zeus::CVector3f::getAngleDiff(normDiff, GetTransform().frontVector());
        zeus::CQuaternion quat = zeus::CQuaternion::lookAt(GetTransform().frontVector(), normDiff,
                                                           std::min(angDif, (dt * x2d4_data.GetTurnSpeed())));

        quat.setImaginary(GetTransform().transposeRotate(quat.getImaginary()));
        RotateInOneFrameOR(quat, dt);
      }

      if (ShouldFire(mgr)) {
        SendScriptMsgs(EScriptObjectState::Attack, mgr, EScriptObjectMessage::None);
        x560_26_firedWithSetBurst = true;
      }

      x52c_curActiveTime = 0.f;
    } else {
      x52c_curActiveTime += dt;
      if (x52c_curActiveTime >= 10.f)
        SetTurretState(ETurretState::ExitTargeting, mgr);
    }
  } else if (msg == EStateMsg::Deactivate) {
    x560_30_needsStopClankSound = true;
  }
}

void CScriptGunTurret::ProcessExitTargettingState(EStateMsg msg, CStateManager& mgr) {
  if (msg != EStateMsg::Update || x25c_gunId == kInvalidUniqueId)
    return;

  if (TCastToPtr<CScriptGunTurret> gun = mgr.ObjectById(x25c_gunId)) {
    //zeus::CTransform gunXf = GetTransform() * GetLocatorTransform("Gun_SDK"sv);

    if (zeus::CVector3f::getAngleDiff(gun->GetTransform().frontVector(), x544_originalFrontVec) < zeus::degToRad(0.9f))
      SetTurretState(ETurretState::Ready, mgr);
  }
}

void CScriptGunTurret::ProcessFrenzyState(EStateMsg msg, CStateManager& mgr, float dt) {
  if (msg == EStateMsg::Activate) {
    x560_31_frenzyReverse = mgr.GetActiveRandom()->Float() < 0.f;
    x534_fireCycleRemTime = 0.15f;
    RemoveMaterial(EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
    mgr.GetPlayer().SetOrbitRequestForTarget(GetUniqueId(), CPlayer::EPlayerOrbitRequest::ActivateOrbitSource, mgr);
  } else if (msg == EStateMsg::Update) {
    if (x524_curStateTime >= x2d4_data.GetFrenzyDuration()) {
      SetTurretState(ETurretState::Destroyed, mgr);
      if (TCastToPtr<CScriptGunTurret> gun = mgr.ObjectById(x25c_gunId))
        gun->x520_state = ETurretState::Destroyed;
      return;
    }

    zeus::CVector3f frontVec = GetTransform().frontVector();
    if (x560_31_frenzyReverse && x550_originalRightVec.magSquared() < 0.f &&
        zeus::CVector3f::getAngleDiff(x544_originalFrontVec, frontVec) >= zeus::degToRad(45.f)) {
      x560_31_frenzyReverse = false;
    } else if (!x560_31_frenzyReverse && x550_originalRightVec.magSquared() < 0.f &&
               zeus::CVector3f::getAngleDiff(x544_originalFrontVec, frontVec) >= zeus::degToRad(45.f)) {
      x560_31_frenzyReverse = true;
    }

    if (TCastToPtr<CScriptGunTurret> gun = mgr.ObjectById(x25c_gunId)) {
      x534_fireCycleRemTime -= dt;
      if (x534_fireCycleRemTime >= 0.f)
        return;

      x404_targetPosition = gun->GetTranslation() + (100.f * gun->GetTransform().frontVector());
      SendScriptMsgs(EScriptObjectState::Attack, mgr, EScriptObjectMessage::None);
      x534_fireCycleRemTime = 0.15f;
    }
  }
}

bool CScriptGunTurret::IsPlayerInFiringRange(CStateManager& mgr) const {
  zeus::CVector3f posDif = mgr.GetPlayer().GetTranslation() - GetTranslation();
  zeus::CVector3f someVec(posDif.x(), posDif.y(), 0.f);
  if (x550_originalRightVec.dot(posDif) >= 0.f)
    return zeus::CVector3f::getAngleDiff(x544_originalFrontVec, someVec) <= x2d4_data.GetRightMaxAngle();

  if (zeus::CVector3f::getAngleDiff(x544_originalFrontVec, someVec) <= x2d4_data.GetLeftMaxAngle())
    return true;

  float biasedAngle = zeus::CVector3f::getAngleDiff(posDif, zeus::skUp) - zeus::degToRad(90.f);

  return (biasedAngle >= zeus::degToRad(-20.f) && biasedAngle <= x2d4_data.GetDownMaxAngle());
}

bool CScriptGunTurret::LineOfSightTest(CStateManager& mgr) const {
  if (x25c_gunId == kInvalidUniqueId)
    return false;
  if (TCastToPtr<CScriptGunTurret> gun = mgr.ObjectById(x25c_gunId)) {
    if (x560_27_burstSet || (x520_state == ETurretState::Inactive && x4a4_extensionModel))
      return true;
    zeus::CTransform xf = GetLocatorTransform("Blast_LCTR"sv);
    zeus::CVector3f muzzlePos = gun->GetTransform().rotate(xf.origin) + gun->GetTranslation();
    zeus::CVector3f dir = mgr.GetPlayer().GetAimPosition(mgr, 0.f) - muzzlePos;
    float mag = dir.magnitude();
    dir = dir / mag;
    rstl::reserved_vector<TUniqueId, 1024> nearList;
    CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid},
                             {EMaterialTypes::Player, EMaterialTypes::CollisionActor});
    mgr.BuildNearList(nearList, muzzlePos, dir, mag, filter, gun.GetPtr());
    TUniqueId id = kInvalidUniqueId;
    return mgr.RayWorldIntersection(id, muzzlePos, dir, mag, filter, nearList).IsInvalid();
  }
  return false;
}

bool CScriptGunTurret::InDetectionRange(CStateManager& mgr) const {
  zeus::CVector3f delta = mgr.GetPlayer().GetTranslation() - GetTranslation();
  if (delta.dot(zeus::skDown) >= 0.f ||
      zeus::CVector3f::getAngleDiff(GetTransform().frontVector(), delta) <= zeus::degToRad(20.f))
    if (delta.magSquared() <= x2d4_data.GetDetectionRange() * x2d4_data.GetDetectionRange())
      if (x2d4_data.GetDetectionZRange() == 0.f || std::fabs(delta.z()) < x2d4_data.GetDetectionZRange())
        return LineOfSightTest(mgr);
  return false;
}

zeus::CVector3f CScriptGunTurret::UpdateExtensionModelState(float dt) {
  if (!x4a4_extensionModel)
    return {};

  switch (x520_state) {
  case ETurretState::PanningA:
  case ETurretState::PanningB:
  case ETurretState::Targeting:
  case ETurretState::Firing:
  case ETurretState::ExitTargeting:
    x4f8_extensionT = std::min(0.9f, x4f8_extensionT + 1.5f * dt);
    break;
  default:
    x4f8_extensionT = std::max(0.f, x4f8_extensionT - 1.5f * dt);
    break;
  case ETurretState::Ready:
  case ETurretState::Deactivating:
  case ETurretState::DeactivatingFromReady:
  case ETurretState::Frenzy:
    break;
  }
  return (x4fc_extensionOffset + (x2d4_data.GetExtensionDropDownDist() * x4f8_extensionT * zeus::skDown)) -
    GetTranslation();
}

void CScriptGunTurret::UpdateHealthInfo(CStateManager& mgr) {
  switch (x258_type) {
  case ETurretComponent::Base:
    if (x25c_gunId != kInvalidUniqueId) {
      if (!TCastToPtr<CScriptGunTurret>(mgr.ObjectById(x25c_gunId))) {
        SetTurretState(ETurretState::Destroyed, mgr);
        x560_25_frozen = false;
        x25c_gunId = kInvalidUniqueId;
        if (x50c_targetingEmitter) {
          CSfxManager::RemoveEmitter(x50c_targetingEmitter);
          x50c_targetingEmitter.reset();
        }
      }
    } else {
      SetTurretState(ETurretState::Destroyed, mgr);
    }
    break;
  case ETurretComponent::Gun:
    if (!x560_24_dead && x520_state != ETurretState::Frenzy && HealthInfo(mgr)->GetHP() <= 0.f) {
      x560_24_dead = true;
      SendScriptMsgs(EScriptObjectState::Dead, mgr, EScriptObjectMessage::None);
      mgr.FreeScriptObject(GetUniqueId());
    }
    break;
  default:
    break;
  }
}

bool CScriptGunTurret::PlayerInsideTurretSphere(CStateManager& mgr) const {
  if (TCastToConstPtr<CCollisionActor> cAct = mgr.GetObjectById(x4a0_collisionActor)) {
    if (cAct->GetActive()) {
      zeus::CVector3f delta = mgr.GetPlayer().GetAimPosition(mgr, 0.f) - GetTranslation();
      if (delta.z() < 0.f) {
        float rad = cAct->GetSphereRadius() * 2.f +
        (cAct->GetTranslation() - GetTranslation()).magnitude();
        return delta.magSquared() < rad * rad;
      }
    }
  }
  return false;
}

void CScriptGunTurret::UpdateGunOrientation(float dt, CStateManager& mgr) {
  if (x25c_gunId == kInvalidUniqueId)
    return;

  if (TCastToPtr<CScriptGunTurret> gun = mgr.ObjectById(x25c_gunId)) {
    zeus::CTransform xf = GetLocatorTransform("Gun_SDK"sv);
    xf = GetTransform() * xf;

    switch (x520_state) {
    case ETurretState::Targeting:
    case ETurretState::Firing: {
      float xyMagSq = xf.frontVector().toVec2f().magSquared();
      float useYaw = 0.f;
      if (std::sqrt(xyMagSq) > 0.001f)
        useYaw = -std::atan2(xf.frontVector().x(), xf.frontVector().y());
      float oldPitch = gun->GetPitch();
      float usePitch = 0.f;
      if (!gun->PlayerInsideTurretSphere(mgr)) {
        zeus::CTransform newXf;
        if ((x404_targetPosition - xf.origin).canBeNormalized())
          newXf = zeus::lookAt(xf.origin, x404_targetPosition);
        else
          newXf = GetTransform();
        float newPitch = -std::atan2(-newXf.frontVector().z(), newXf.frontVector().toVec2f().magnitude());
        float newPitchDelta = newPitch - oldPitch;
        float f2 = newPitchDelta > 0.f ? dt * x2d4_data.GetTurnSpeed() : dt * -x2d4_data.GetTurnSpeed();
        usePitch = std::max(std::fabs(newPitchDelta) <=
          std::fabs(f2) ? newPitch : oldPitch + f2, -x2d4_data.GetDownMaxAngle());
      }
      zeus::CQuaternion qy, qx, qz;
      qy.rotateY(0.f);
      qx.rotateX(usePitch);
      qz.rotateZ(useYaw);
      gun->SetTransform((qz * qx * qy).toTransform(xf.origin));
      break;
    }
    case ETurretState::ExitTargeting: {
      zeus::CVector3f frontVec = GetTransform().frontVector();
      zeus::CVector3f gunFrontVec = gun->GetTransform().frontVector();
      float rotAngle = 0.3f * dt * x2d4_data.GetTurnSpeed();
      zeus::CQuaternion quat = zeus::CQuaternion::lookAt(gunFrontVec, frontVec, rotAngle);
      quat.setImaginary(gun->GetTransform().transposeRotate(quat.getImaginary()));
      gun->RotateInOneFrameOR(quat, dt);
      zeus::CQuaternion quat2 = zeus::CQuaternion::lookAt(frontVec, x544_originalFrontVec, rotAngle);
      quat2.setImaginary(GetTransform().transposeRotate(quat2.getImaginary()));
      RotateInOneFrameOR(quat2, dt);
      break;
    }
    case ETurretState::Frenzy: {
      float xyMagSq = xf.frontVector().toVec2f().magSquared();
      float useYaw = 0.f;
      if (std::sqrt(xyMagSq) > 0.001f)
        useYaw = -std::atan2(xf.frontVector().x(), xf.frontVector().y());
      float f28 = -0.5f * x2d4_data.GetDownMaxAngle() *
        (1.f - std::cos(2.f * x524_curStateTime * x2d4_data.GetTurnSpeed()));
      float pitch = gun->GetPitch();
      float f2 = f28 - pitch;
      float f31 = x2d4_data.GetTurnSpeed() * dt;
      float f3 = f2 > 0.f ? f31 : -f31;
      float usePitch = std::fabs(f2) <= std::fabs(f3) ? f28 : pitch + f3;
      usePitch = std::max(usePitch, -x2d4_data.GetDownMaxAngle());
      zeus::CQuaternion qy, qx, qz;
      qy.rotateY(0.f);
      qx.rotateX(usePitch);
      qz.rotateZ(useYaw);
      gun->SetTransform((qz * qx * qy).toTransform(xf.origin));
      zeus::CQuaternion rot = zeus::CQuaternion::lookAt(GetTransform().frontVector(),
        x560_31_frenzyReverse ? -x550_originalRightVec : x550_originalRightVec, f31);
      rot.setImaginary(GetTransform().transposeRotate(rot.getImaginary()));
      RotateInOneFrameOR(rot, dt);
      break;
    }
    default:
      gun->SetTransform(xf);
      break;
    }
  }
}

void CScriptGunTurret::UpdateTargettingSound(float dt) {
  x510_timeSinceLastTargetSfx += dt;
  float angleDiff2D = zeus::CVector2f::getAngleDiff(x514_lastFrontVector.toVec2f(),
                                                    GetTransform().frontVector().toVec2f());

  if (x560_30_needsStopClankSound && angleDiff2D < zeus::degToRad(20.f) &&
      (x520_state == ETurretState::Targeting || x520_state == ETurretState::Firing)) {
    if (!x560_25_frozen)
      CSfxManager::AddEmitter(x2d4_data.GetStopClankSoundId(), GetTranslation(), zeus::skUp, false, false,
                              127, GetAreaIdAlways());
    x560_30_needsStopClankSound = false;
  }

  if (x510_timeSinceLastTargetSfx >= 0.5f && !x560_25_frozen) {
    if (x520_state == ETurretState::Targeting || x520_state == ETurretState::Firing ||
        x520_state == ETurretState::Frenzy) {
      bool insignificant = IsInsignificantRotation(dt);
      if (!insignificant && !x50c_targetingEmitter)
        x50c_targetingEmitter = CSfxManager::AddEmitter(x2d4_data.GetTrackingSoundId(), GetTranslation(),
                                                        zeus::skUp, false, true, 127, GetAreaIdAlways());
      else if (insignificant && x50c_targetingEmitter) {
        CSfxManager::RemoveEmitter(x50c_targetingEmitter);
        x50c_targetingEmitter.reset();
        x510_timeSinceLastTargetSfx = 0.f;
      }

      if (x50c_targetingEmitter) {
        float bendScale = dt * x2d4_data.GetTurnSpeed();
        CSfxManager::PitchBend(x50c_targetingEmitter,
          std::max(0.f, (bendScale > 0.f ? angleDiff2D / bendScale : 0.f)));
      }
    }
  } else if (x560_25_frozen && x50c_targetingEmitter) {
    CSfxManager::RemoveEmitter(x50c_targetingEmitter);
    x50c_targetingEmitter.reset();
  }

  x514_lastFrontVector = GetTransform().frontVector();
}

void CScriptGunTurret::PlayAdditiveChargingAnimation(CStateManager& mgr) {
  if (x520_state == ETurretState::Firing) {
    if (x55c_additiveChargeAnim != -1)
      return;

    auto pair = GetModelData()->GetAnimationData()->GetCharacterInfo().GetPASDatabase().FindBestAnimation(
        CPASAnimParmData(24, CPASAnimParm::FromEnum(2)), *mgr.GetActiveRandom(), -1);
    if (pair.first > 0.f) {
      x55c_additiveChargeAnim = pair.second;
      GetModelData()->GetAnimationData()->AddAdditiveAnimation(pair.second, 1.f, true, false);
    }
  } else if (x55c_additiveChargeAnim != -1) {
    GetModelData()->GetAnimationData()->DelAdditiveAnimation(x55c_additiveChargeAnim);
    x55c_additiveChargeAnim = -1;
  }
}

void CScriptGunTurret::UpdateTargettingMode(float dt, CStateManager& mgr) {
  if (mgr.GetCameraManager()->IsInCinematicCamera()) {
    x534_fireCycleRemTime = mgr.GetActiveRandom()->Float() * x2d4_data.GetReloadTimeVariance() +
      x2d4_data.GetReloadTime();
    x538_halfFireCycleDur = 0.5f * x534_fireCycleRemTime;
  }

  if (x534_fireCycleRemTime > 0.f) {
    x534_fireCycleRemTime -= dt;
    if (x534_fireCycleRemTime < x538_halfFireCycleDur && x520_state != ETurretState::Firing) {
      CSfxManager::AddEmitter(x2d4_data.GetChargingSoundId(), GetTranslation(), zeus::skUp, false, false,
                              0x7f, GetAreaIdAlways());
      SetTurretState(ETurretState::Firing, mgr);
      return;
    }
  } else {
    if (x520_state != ETurretState::Targeting)
      SetTurretState(ETurretState::Targeting, mgr);

    if (!x3a4_burstFire.IsBurstSet()) {
      UpdateBurstType(mgr);
      x534_fireCycleRemTime = mgr.GetActiveRandom()->Float() * x2d4_data.GetReloadTimeVariance() +
        x2d4_data.GetReloadTime();
      x538_halfFireCycleDur = 0.5f * x534_fireCycleRemTime;
    } else {
      x3a4_burstFire.Update(mgr, dt);
    }
  }
}

void CScriptGunTurret::UpdateBurstType(CStateManager& mgr) {
  if (x560_27_burstSet) {
    bool inView = true;
    if (mgr.GetPlayer().GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed) {
      zeus::CVector3f frontVec = GetTransform().frontVector();
      zeus::CVector3f plFrontVec = mgr.GetPlayer().GetTransform().frontVector();
      float dot = frontVec.dot(plFrontVec);

      if (dot >= 0.f)
        inView = false;
    }
    u32 r3 = mgr.GetActiveRandom()->Range(0, 3);
    r3 += 2;
    u32 type;
    if (r3 <= 2 || x2d4_data.GetNumSubsequentShots() < 3)
      type = 0;
    else if (r3 >= 5 && x2d4_data.GetNumSubsequentShots() > 3)
      type = 2;
    else
      type = 1;

    x3a4_burstFire.SetBurstType(type + (inView ? 0 : 3));
  } else {
    u32 type = x2d4_data.GetNumInitialShots() - 2;
    x3a4_burstFire.SetBurstType(type);
    x3a4_burstFire.SetFirstBurstIndex(x2d4_data.GetInitialShotTableIndex());
  }

  x3a4_burstFire.Start(mgr);
  x560_26_firedWithSetBurst = false;
  x560_27_burstSet = true;
}

bool CScriptGunTurret::ShouldFire(CStateManager& mgr) const {
  if (x520_state == ETurretState::Targeting && x534_fireCycleRemTime <= 0.f && x3a4_burstFire.ShouldFire())
    return IsPlayerInFiringRange(mgr);

  return false;
}

bool CScriptGunTurret::IsInsignificantRotation(float dt) const {
  return zeus::CVector2f::getAngleDiff(x514_lastFrontVector.toVec2f(), GetTransform().frontVector().toVec2f()) <
         zeus::degToRad(2.f) * dt;
}

} // namespace urde
