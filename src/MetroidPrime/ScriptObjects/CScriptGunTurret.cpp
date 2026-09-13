#include "MetroidPrime/ScriptObjects/CScriptGunTurret.hpp"

#include "Collision/CRayCastResult.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Enemies/CAiFuncMap.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"

#include <Kyoto/Animation/CPASAnimParmData.hpp>
#include <Kyoto/Audio/CSfxManager.hpp>
#include <Kyoto/Basics/CCast.hpp>
#include <Kyoto/Math/CRelAngle.hpp>
#include <Kyoto/Particles/CElementGen.hpp>
#include <Kyoto/Particles/CGenDescription.hpp>

const uint CScriptGunTurret::skStateToLocoTypeLookup[] = {
    5, 7, 9, 0, 1, 0, 1, 2, 3, 1, 1, 1, 1,
};

const SBurst CScriptGunTurret::skBurst2InfoTemplate[] = {
    {3, {1, 2, -1, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {3, {7, 6, -1, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {4, {3, 5, -1, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {60, {16, 4, -1, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {30, {4, 4, -1, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
};

const SBurst CScriptGunTurret::skBurst3InfoTemplate[] = {
    {30, {4, 5, 4, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {30, {2, 3, 4, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {30, {3, 4, 5, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {5, {16, 1, 2, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {5, {8, 7, 6, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
};

const SBurst CScriptGunTurret::skBurst4InfoTemplate[] = {
    {5, {16, 1, 2, 3, 0, 0, 0, 0}, 0.150000, 0.050000},
    {5, {9, 8, 7, 6, 0, 0, 0, 0}, 0.150000, 0.050000},
    {15, {2, 3, 4, 5, 0, 0, 0, 0}, 0.150000, 0.050000},
    {15, {5, 4, 3, 2, 0, 0, 0, 0}, 0.150000, 0.050000},
    {15, {10, 11, 4, 13, 0, 0, 0, 0}, 0.150000, 0.050000},
    {15, {14, 13, 4, 11, 0, 0, 0, 0}, 0.150000, 0.050000},
    {30, {2, 4, 4, 6, 0, 0, 0, 0}, 0.150000, 0.050000},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
};

const SBurst CScriptGunTurret::skOOVBurst2InfoTemplate[] = {
    {20, {16, 15, -1, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {20, {8, 9, -1, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {20, {13, 11, -1, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {20, {2, 6, -1, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {20, {3, 4, -1, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
};

const SBurst CScriptGunTurret::skOOVBurst3InfoTemplate[] = {
    {10, {14, 4, 10, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {10, {15, 13, 4, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {10, {9, 11, 4, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {35, {15, 13, 11, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {35, {9, 11, 13, -1, 0, 0, 0, 0}, 0.150000, 0.050000},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
};

const SBurst CScriptGunTurret::skOOVBurst4InfoTemplate[] = {
    {10, {14, 13, 4, 11, 0, 0, 0, 0}, 0.150000, 0.050000},
    {30, {1, 15, 13, 11, 0, 0, 0, 0}, 0.150000, 0.050000},
    {20, {16, 15, 14, 13, 0, 0, 0, 0}, 0.150000, 0.050000},
    {10, {8, 9, 11, 4, 0, 0, 0, 0}, 0.150000, 0.050000},
    {10, {1, 15, 13, 4, 0, 0, 0, 0}, 0.150000, 0.050000},
    {20, {8, 9, 10, 11, 0, 0, 0, 0}, 0.150000, 0.050000},
    {0, {0, 0, 0, 0, 0, 0, 0, 0}, 0.000000, 0.000000},
};

const int CScriptGunTurretData::skMinProperties = 43;
const float CScriptGunTurret::skExtensionOverlapMaxPer = 0.89999999f;
const char* const CScriptGunTurret::skGunLCTRName = "Gun_SDK";
const char* const CScriptGunTurret::skBlastLCTRName = "Blast_LCTR";
const char* const CScriptGunTurret::skLightLCTRName = "light_LCTR";
const char* const CScriptGunTurret::skLockonTargetLCTRName = "lockon_target_LCTR";

const SBurst* CScriptGunTurret::skBursts[] = {
    skBurst2InfoTemplate,
    skBurst3InfoTemplate,
    skBurst4InfoTemplate,
    skOOVBurst2InfoTemplate,
    skOOVBurst3InfoTemplate,
    skOOVBurst4InfoTemplate,
    nullptr,
};

CScriptGunTurretData::CScriptGunTurretData(CInputStream& in, const int propCount)
: x0_intoDeactivateDelay(in.Get< float >())
, x4_intoActivateDelay(in.Get< float >())
, x8_reloadTime(in.Get< float >())
, xc_reloadTimeVariance(in.Get< float >())
, x10_panStartTime(in.Get< float >())
, x14_panHoldTime(in.Get< float >())
, x18_totalPanSearchTime(30.f)
, x1c_leftMaxAngle(CRelAngle::FromDegrees(in.Get< float >()).AsRadians())
, x20_rightMaxAngle(CRelAngle::FromDegrees(in.Get< float >()).AsRadians())
, x24_downMaxAngle(CRelAngle::FromDegrees(in.Get< float >()).AsRadians())
, x28_turnSpeed(CRelAngle::FromDegrees(in.Get< float >()).AsRadians())
, x2c_detectionRange(in.Get< float >())
, x30_detectionZRange(in.Get< float >())
, x34_freezeDuration(in.Get< float >())
, x38_freezeVariance(in.Get< float >())
, x3c_freezeTimeout(propCount >= 48 ? in.Get< bool >() : false)
, x40_projectileRes(in.Get< CAssetId >())
, x44_projectileDamage(in)
, x60_idleLightRes(in.Get< CAssetId >())
, x64_deactivateLightRes(in.Get< CAssetId >())
, x68_targettingLightRes(in.Get< CAssetId >())
, x6c_frozenEffectRes(in.Get< CAssetId >())
, x70_chargingEffectRes(in.Get< CAssetId >())
, x74_panningEffectRes(in.Get< CAssetId >())
, x78_visorEffectRes(propCount >= 44 ? in.Get< CAssetId >() : kInvalidAssetId)
, x7c_trackingSoundId(CSfxManager::TranslateSFXID(in.Get< int >()))
, x7e_lockOnSoundId(CSfxManager::TranslateSFXID(in.Get< int >()))
, x80_unfreezeSoundId(CSfxManager::TranslateSFXID(in.Get< int >()))
, x82_stopClankSoundId(CSfxManager::TranslateSFXID(in.Get< int >()))
, x84_chargingSoundId(CSfxManager::TranslateSFXID(in.Get< int >()))
, x86_visorSoundId(propCount >= 45 ? CSfxManager::TranslateSFXID(in.Get< int >())
                                   : CSfxManager::kInternalInvalidSfxId)
, x88_extensionModelResId(in.Get< CAssetId >())
, x8c_extensionDropDownDist(in.Get< float >())
, x90_numInitialShots(in.Get< int >())
, x94_initialShotTableIndex(in.Get< int >())
, x98_numSubsequentShots(in.Get< int >())
, x9c_frenzyDuration(propCount >= 47 ? in.Get< float >() : 3.f)
, xa0_scriptedStartOnly(propCount >= 46 ? in.Get< bool >() : false) {}

static const CMaterialList skTurretMaterialList(kMT_Character);
static const CMaterialList skGunMaterialList(kMT_Solid, kMT_Character, kMT_Orbit, kMT_Target);

CScriptGunTurret::CScriptGunTurret(TUniqueId uid, const rstl::string& name, ETurretComponent comp,
                                   const CEntityInfo& info, const CTransform4f& xf,
                                   const CModelData& mData, const CAABox& aabb,
                                   const CHealthInfo& hInfo, const CDamageVulnerability& dVuln,
                                   const CActorParameters& aParms,
                                   const CScriptGunTurretData& turretData)
: CPhysicsActor(uid, true, name, info, xf, mData,
                comp == kTC_Base ? skTurretMaterialList : skGunMaterialList, aabb,
                SMoverData(1000.f), aParms, 0.3f, 0.1f)
, x258_type(comp)
, x25c_gunId(kInvalidUniqueId)
, x260_lastGunHP(0.f)
, x264_healthInfo(hInfo)
, x26c_damageVuln(dVuln)
, x2d4_data(turretData)
, x378_(kInvalidUniqueId)
, x37c_projectileInfo(turretData.GetProjectileRes(), turretData.GetProjectileDamage())
, x3a4_burstFire(skBursts, 1)
, x404_targetPosition(CVector3f::Zero())
, x410_idleLightDesc(gpSimplePool->GetObj(SObjectTag('PART', turretData.GetIdleLightRes())))
, x41c_deactivateLightDesc(
      gpSimplePool->GetObj(SObjectTag('PART', turretData.GetDeactivateLightRes())))
, x428_targettingLightDesc(
      gpSimplePool->GetObj(SObjectTag('PART', turretData.GetTargettingLightRes())))
, x434_frozenEffectDesc(gpSimplePool->GetObj(SObjectTag('PART', turretData.GetFrozenEffectRes())))
, x440_chargingEffectDesc(
      gpSimplePool->GetObj(SObjectTag('PART', turretData.GetChargingEffectRes())))
, x44c_panningEffectDesc(gpSimplePool->GetObj(SObjectTag('PART', turretData.GetPanningEffectRes())))
, x458_visorEffectDesc(
      turretData.GetVisorEffectRes() != kInvalidAssetId
          ? rstl::optional_object< TLockedToken< CGenDescription > >(
                gpSimplePool->GetObj(SObjectTag('PART', turretData.GetVisorEffectRes())))
          : rstl::optional_object_null())
, x468_idleLight(rs_new CElementGen(x410_idleLightDesc))
, x470_deactivateLight(rs_new CElementGen(x41c_deactivateLightDesc))
, x478_targettingLight(rs_new CElementGen(x428_targettingLightDesc))
, x480_frozenEffect(rs_new CElementGen(x434_frozenEffectDesc))
, x488_chargingEffect(rs_new CElementGen(x440_chargingEffectDesc))
, x490_panningEffect(rs_new CElementGen(x44c_panningEffectDesc))
, x498_lightId(kInvalidUniqueId)
, x4a0_collisionActor(kInvalidUniqueId)
, x4a4_extensionModel(rstl::optional_object< CModelData >())
, x4f4_extensionRange(0.f)
, x4f8_extensionT(0.f)
, x4fc_extensionOffset(xf.GetTranslation())
, x508_gunSDKSeg(CSegId::Invalid())
, x510_timeSinceLastTargetSfx(0.f)
, x514_lastFrontVector(xf.GetForward())
, x520_state(kTS_Invalid)
, x524_curStateTime(0.f)
, x528_curInactiveTime(0.f)
, x52c_curActiveTime(0.f)
, x530_curPanTime(0.f)
, x534_fireCycleRemTime(0.f)
, x538_halfFireCycleDur(0.f)
, x53c_freezeRemTime(0.f)
, x540_turretAnim(-1)
, x544_originalFrontVec(xf.GetForward())
, x550_originalRightVec(xf.GetRight())
, x55c_additiveChargeAnim(-1)
, x560_24_dead(false)
, x560_25_frozen(false)
, x560_26_firedWithSetBurst(false)
, x560_27_burstSet(false)
, x560_28_hasBeenActivated(false)
, x560_29_scriptedStart(false)
, x560_30_needsStopClankSound(true)
, x560_31_frenzyReverse(false) {

  if (comp == kTC_Base && HasAnimation()) {
    ModelData()->EnableLooping(true);
  }
  x37c_projectileInfo.Token().Lock();
}

CScriptGunTurret::~CScriptGunTurret() {}

void CScriptGunTurret::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  switch (x258_type) {
  case kTC_Base:
    if (!x560_25_frozen) {
      ProcessGunStateMachine(dt, mgr);
      UpdateTurretAnimation();
      UpdateGunOrientation(dt, mgr);
      CVector3f vec = UpdateExtensionModelState(dt);
      const CAdvancementDeltas deltas = UpdateAnimation(dt, mgr, true);
      SetTranslation((GetTranslation() + deltas.GetOffsetDelta()) + vec);
      RotateToOR(deltas.GetOrientationDelta(), dt);
    } else {
      Stop();
    }

    UpdateTargettingSound(dt);
    break;
  case kTC_Gun:
    UpdateGunParticles(dt, mgr);
    const CAdvancementDeltas deltas = UpdateAnimation(dt, mgr, true);
    MoveToOR(deltas.GetOffsetDelta(), dt);
    RotateToOR(deltas.GetOrientationDelta(), dt);
    UpdateGunCollisionManager(dt, mgr);
    UpdateFrozenState(dt, mgr);
    break;
  }

  UpdateHealthInfo(mgr);
}

void CScriptGunTurret::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                       CStateManager& mgr) {
  CPhysicsActor::AcceptScriptMsg(msg, uid, mgr);

  switch (msg) {
  case kSM_Activate: {
    if (!x49c_collisionManager.null()) {
      x49c_collisionManager->SetActive(mgr, true);
    }
    break;
  }
  case kSM_Deactivate: {
    if (!x49c_collisionManager.null()) {
      x49c_collisionManager->SetActive(mgr, false);
    }
    break;
  }
  case kSM_Registered: {
    if (x258_type == kTC_Gun && x478_targettingLight->SystemHasLight()) {
      x498_lightId = mgr.AllocateUniqueId();
      mgr.AddObject(rs_new CGameLight(x498_lightId, GetCurrentAreaId(), GetActive(),
                                      rstl::string_l("ParticleLight_") + GetDebugName(),
                                      GetTransform(), GetUniqueId(),
                                      x478_targettingLight->GetLight(), 0, 1, 0.f));
      SetupCollisionManager(mgr);
    } else if (x258_type == kTC_Base) {
      if (x2d4_data.GetExtensionModelResId() != kInvalidAssetId) {
        x4a4_extensionModel =
            rstl::optional_object< CModelData >(SetupExtensionModel(GetModelScale()));
        x4f4_extensionRange = x4a4_extensionModel->GetBounds().GetDepth();
      }

      SetTurretState(kTS_Inactive, mgr);
    }
    break;
  }

  case kSM_Deleted: {
    if (x258_type == kTC_Gun && x498_lightId != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(x498_lightId);
    }

    if (x50c_targetingEmitter) {
      CSfxManager::RemoveEmitter(x50c_targetingEmitter);
    }

    if (!x49c_collisionManager.null()) {
      x49c_collisionManager->Destroy(mgr);
    }
    break;
  }
  case kSM_Start:
    if (x258_type != kTC_Base || x520_state != kTS_Inactive) {
      break;
    }
    x560_29_scriptedStart = true;
    break;
  case kSM_Stop: {
    if (x258_type != kTC_Base) {
      break;
    }

    if (x520_state != kTS_Deactivate && x520_state != kTS_DeactivateFromReady &&
        x520_state != kTS_Deactivating) {
      SetTurretState(x560_28_hasBeenActivated ? kTS_DeactivatingFromReady : kTS_Deactivating, mgr);
    }
  } break;
  case kSM_Action: {
    if (x258_type == kTC_Gun) {
      LaunchProjectile(mgr);
    } else if (x258_type == kTC_Base) {
      PlayAdditiveFlinchAnimation(mgr);
    }
  } break;
  case kSM_SetToMax: {
    x560_25_frozen = false;
    SetMuted(false);
  } break;
  case kSM_SetToZero: {
    x560_25_frozen = true;
    SetMuted(true);
  } break;
  case kSM_InitializedInArea: {
    if (x258_type != kTC_Base) {
      break;
    }

    for (AUTO(conn, GetConnectionList().begin()); conn != GetConnectionList().end(); ++conn) {
      if (conn->x0_state != kSS_Play || conn->x4_msg != kSM_Activate) {
        continue;
      }

      TUniqueId gunId = mgr.GetIdForScript(conn->x8_objId);

      if (CScriptGunTurret* gun = TCastToPtr< CScriptGunTurret >(mgr.ObjectById(gunId))) {
        x25c_gunId = gunId;
        x260_lastGunHP = gun->GetHealthInfo(mgr)->GetHP();
        break;
      }
    }
  } break;
  case kSM_Damage: {
    if (x258_type != kTC_Gun) {
      break;
    }
    if (!(GetHealthInfo(mgr)->GetHP() <= 0.f)) {
      break;
    }

    if (const CGameProjectile* proj = TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(uid))) {
      if ((proj->GetAttribField() & CWeapon::kPA_Wave) != CWeapon::kPA_Wave) {
        break;
      }

      x520_state = kTS_Frenzy;
      RemoveMaterial(kMT_Target, kMT_Orbit, mgr);
      mgr.Player()->TryToBreakOrbit(GetUniqueId(), CPlayer::kOB_ActivateOrbitSource, mgr);
      x53c_freezeRemTime = 0.f;
    }
  } break;
  default:
    break;
  }
}

ENTITY_ACCEPT_IMPL(CScriptGunTurret)

void CScriptGunTurret::Render(const CStateManager& mgr) const {
  CPhysicsActor::Render(mgr);

  if (x258_type == kTC_Gun) {
    if (!x560_25_frozen) {
      switch (x520_state) {
      case kTS_Deactivate:
      case kTS_DeactivateFromReady:
      case kTS_Deactivating:
      case kTS_DeactivatingFromReady:
        x470_deactivateLight->Render();
        break;
      case kTS_Inactive:
        x468_idleLight->Render();
        break;
      case kTS_PanningA:
      case kTS_PanningB:
        x490_panningEffect->Render();
        break;
      case kTS_Ready:
      case kTS_Targeting:
      case kTS_Firing:
      case kTS_ExitTargeting:
      case kTS_Frenzy:
        x478_targettingLight->Render();
        if (x520_state == kTS_Firing) {
          x488_chargingEffect->Render();
        }
        break;
      default:
        break;
      }
    } else {
      x480_frozenEffect->Render();
    }
  } else if (x258_type == kTC_Base && x4a4_extensionModel && x4f8_extensionT > 0.f) {
    CTransform4f xf = GetTransform();
    xf.SetTranslation(x4fc_extensionOffset + x4f4_extensionRange * (CVector3f::Down() * 0.5f));
    x4a4_extensionModel->Render(mgr, xf, GetActorLights(), CModelFlags::Normal());
  }
}

void CScriptGunTurret::AddToRenderer(const CFrustumPlanes& frustum,
                                     const CStateManager& mgr) const {
  CActor::AddToRenderer(frustum, mgr);

  if (x258_type != kTC_Gun) {
    return;
  }

  if (!x560_25_frozen) {
    switch (x520_state) {
    case kTS_Deactivate:
    case kTS_DeactivateFromReady:
    case kTS_Deactivating:
    case kTS_DeactivatingFromReady:
      gpRender->AddParticleGen(*x470_deactivateLight);
      break;
    case kTS_Inactive:
      gpRender->AddParticleGen(*x468_idleLight);
      break;
    case kTS_PanningA:
    case kTS_PanningB:
      gpRender->AddParticleGen(*x490_panningEffect);
      break;
    case kTS_Ready:
    case kTS_Targeting:
    case kTS_Firing:
    case kTS_ExitTargeting:
    case kTS_Frenzy:
      gpRender->AddParticleGen(*x478_targettingLight);
      if (x520_state == kTS_Firing || x520_state == kTS_Frenzy) {
        gpRender->AddParticleGen(*x488_chargingEffect);
      }
      break;
    default:
      break;
    }
  } else {
    gpRender->AddParticleGen(*x480_frozenEffect);
  }
}

rstl::optional_object< CAABox > CScriptGunTurret::GetTouchBounds() const {
  if (GetActive() && GetMaterialList().HasMaterial(kMT_Solid)) {
    return GetBoundingBox();
  }
  return rstl::optional_object_null();
}

void CScriptGunTurret::Touch(CActor& actor, CStateManager& mgr) {
  if (x258_type != kTC_Gun) {
    return;
  }

  if (const CGameProjectile* proj = TCastToConstPtr< CGameProjectile >(actor)) {
    const CPlayer* player = mgr.GetPlayer();
    if (proj->GetOwnerId() == player->GetUniqueId()) {
      const CDamageVulnerability* dVuln = GetDamageVulnerability();
      if (!x560_24_dead && x520_state != kTS_Frenzy &&
          (proj->GetAttribField() & CWeapon::kPA_Ice) == CWeapon::kPA_Ice &&
          dVuln->WeaponHits(CWeaponMode::Ice(), CDamageVulnerability::kRD_No)) {
        x560_25_frozen = true;
        SendScriptMsgs(kSS_Zero, mgr, kSM_None);
        x53c_freezeRemTime =
            mgr.Random()->Float() * x2d4_data.GetFreezeVariance() + x2d4_data.GetFreezeDuration();
        SetMuted(true);
      }

      SendScriptMsgs(kSS_Damage, mgr, kSM_None);
    }
  }
}

CVector3f CScriptGunTurret::GetAimPosition(const CStateManager& mgr, float dt) const {
  if (x258_type == kTC_Gun) {
    CTransform4f lctrXf(GetLocatorTransform(rstl::string_l(skGunLCTRName)));
    return GetTranslation() + GetTransform().Rotate(lctrXf.GetTranslation());
  }

  return GetTranslation();
}

CVector3f CScriptGunTurret::GetOrbitPosition(const CStateManager& mgr) const {
  return GetAimPosition(mgr, 0.f);
}

void CScriptGunTurret::SetTurretState(const ETurretState state, CStateManager& mgr) {
  if (state >= kTS_Destroyed && state <= kTS_Frenzy) {
    if (x520_state != kTS_Invalid) {
      ProcessCurrentState(kStateMsg_Deactivate, 0.f, mgr);
    }

    x520_state = state;
    x524_curStateTime = 0.f;
    ProcessCurrentState(kStateMsg_Activate, 0.f, mgr);
  }
}

void CScriptGunTurret::ProcessGunStateMachine(float dt, CStateManager& mgr) {
  ProcessCurrentState(kStateMsg_Update, dt, mgr);
  x524_curStateTime += dt;
  PlayAdditiveChargingAnimation(mgr);

  if (x25c_gunId == kInvalidUniqueId) {
    return;
  }

  CScriptGunTurret* gun = TCastToPtr< CScriptGunTurret >(mgr.ObjectById(x25c_gunId));
  if (!gun) {
    return;
  }

  if (gun->x520_state != kTS_Frenzy) {
    gun->x520_state = x520_state;
    return;
  }

  if (x520_state == kTS_Frenzy) {
    return;
  }

  SetTurretState(kTS_Frenzy, mgr);
  gun->RemoveMaterial(kMT_Target, kMT_Orbit, mgr);
  mgr.Player()->TryToBreakOrbit(GetUniqueId(), CPlayer::kOB_ActivateOrbitSource, mgr);
}

void CScriptGunTurret::ProcessCurrentState(EStateMsg msg, float dt, CStateManager& mgr) {
  switch (x520_state) {
  case kTS_Destroyed:
    break;
  case kTS_Deactivate:
    break;
  case kTS_Deactivating:
  case kTS_DeactivatingFromReady:
    ProcessDeactivatingState(msg, dt, mgr);
    break;
  case kTS_Inactive:
    ProcessInactiveState(msg, dt, mgr);
    break;
  case kTS_Ready:
    ProcessReadyState(msg, dt, mgr);
    break;
  case kTS_PanningA:
  case kTS_PanningB:
    ProcessPanningState(msg, dt, mgr);
    break;
  case kTS_Targeting:
  case kTS_Firing:
    ProcessTargettingState(msg, dt, mgr);
    break;
  case kTS_ExitTargeting:
    ProcessExitTargettingState(msg, dt, mgr);
    break;
  case kTS_Frenzy:
    ProcessFrenzyState(msg, dt, mgr);
    break;
  default:
    break;
  }
}

void CScriptGunTurret::ProcessDeactivatingState(EStateMsg msg, float dt, CStateManager& mgr) {
  switch (msg) {
  case kStateMsg_Update:
    if (x524_curStateTime >= x2d4_data.GetIntoDeactivateDelay()) {
      SetTurretState(x560_28_hasBeenActivated ? kTS_DeactivateFromReady : kTS_Deactivate, mgr);
    }
    break;
  default:
    break;
  }
}

void CScriptGunTurret::ProcessInactiveState(EStateMsg msg, float dt, CStateManager& mgr) {
  switch (msg) {
  case kStateMsg_Activate:
    x528_curInactiveTime = 0.f;
    x560_27_burstSet = false;
    if (CScriptGunTurret* gun = TCastToPtr< CScriptGunTurret >(mgr.ObjectById(x25c_gunId))) {
      x260_lastGunHP = gun->GetHealthInfo(mgr)->GetHP();
    }
    break;
  case kStateMsg_Update: {
    bool forceActivate = false;
    if (x25c_gunId != kInvalidUniqueId) {
      if (CScriptGunTurret* gun = TCastToPtr< CScriptGunTurret >(mgr.ObjectById(x25c_gunId))) {
        forceActivate = gun->GetHealthInfo(mgr)->GetHP() < x260_lastGunHP;
      }
    }

    if (x2d4_data.IsScriptedStartOnly()
            ? (forceActivate || x560_29_scriptedStart)
            : (forceActivate || x560_29_scriptedStart || InDetectionRange(mgr))) {
      x528_curInactiveTime += dt;
      if (forceActivate || x528_curInactiveTime >= x2d4_data.GetIntoActivateDelay()) {
        SetTurretState(kTS_Ready, mgr);
      }
    } else {
      x468_idleLight->SetParticleEmission(true);
    }
  } break;
  case kStateMsg_Deactivate:
    x560_28_hasBeenActivated = true;
    x468_idleLight->SetParticleEmission(false);
    if (CScriptGunTurret* gun = TCastToPtr< CScriptGunTurret >(mgr.ObjectById(x25c_gunId))) {
      x260_lastGunHP = gun->GetHealthInfo(mgr)->GetHP();
    }
    break;
  }
}

void CScriptGunTurret::ProcessReadyState(EStateMsg msg, float dt, CStateManager& mgr) {
  switch (msg) {
  case kStateMsg_Activate:
    x52c_curActiveTime = 0.f;
    break;
  case kStateMsg_Update:
    x52c_curActiveTime += dt;
    if (x52c_curActiveTime >= x2d4_data.GetPanStartTime()) {
      if (IsPlayerInFiringRange(mgr) && InDetectionRange(mgr)) {
        SetTurretState(kTS_Targeting, mgr);
        CSfxManager::AddEmitter(x2d4_data.GetLockOnSoundId(), GetTranslation(), CVector3f::Up(),
                                false, false, CSfxManager::kMedPriority,
                                GetCurrentAreaId().Value());
      } else {
        SetTurretState(kTS_PanningA, mgr);
        x530_curPanTime = 0.f;
      }
    }
    break;
  default:
    break;
  }
}

void CScriptGunTurret::ProcessPanningState(EStateMsg msg, float dt, CStateManager& mgr) {
  switch (msg) {
  case kStateMsg_Activate:
    x52c_curActiveTime = 0.f;
    break;
  case kStateMsg_Update:
    if (IsPlayerInFiringRange(mgr) && InDetectionRange(mgr)) {
      SetTurretState(kTS_Targeting, mgr);
      CSfxManager::AddEmitter(x2d4_data.GetLockOnSoundId(), GetTranslation(), CVector3f::Up(),
                              false, false, CSfxManager::kMedPriority, GetCurrentAreaId().Value());
    } else {
      x52c_curActiveTime += dt;
      x530_curPanTime += dt;
      if (x530_curPanTime >= x2d4_data.GetPanTotalSearchTime() && !x4a4_extensionModel &&
          !x2d4_data.IsScriptedStartOnly()) {
        SetTurretState(kTS_Inactive, mgr);
        x560_29_scriptedStart = x560_28_hasBeenActivated = false;
      } else if (x52c_curActiveTime >= x2d4_data.GetPanHoldTime()) {
        SetTurretState(x520_state == kTS_PanningA ? kTS_PanningB : kTS_PanningA, mgr);
      }
    }
    break;
  default:
    break;
  }
}

void CScriptGunTurret::ProcessTargettingState(EStateMsg msg, float dt, CStateManager& mgr) {
  switch (msg) {
  case kStateMsg_Activate:
    x52c_curActiveTime = 0.f;
    break;
  case kStateMsg_Update:
    if (x560_26_firedWithSetBurst || InDetectionRange(mgr)) {
      UpdateTargettingMode(dt, mgr);
      if (x25c_gunId != kInvalidUniqueId) {
        if (CScriptGunTurret* const gun =
                TCastToPtr< CScriptGunTurret >(mgr.ObjectById(x25c_gunId))) {
          CVector3f intercept = x404_targetPosition;
          if (IsPlayerInFiringRange(mgr)) {
            const CTransform4f blastXf = gun->GetLocatorTransform(rstl::string_l(skBlastLCTRName));
            const CVector3f blastPos =
                gun->GetTranslation() + gun->GetTransform().Rotate(blastXf.GetTranslation());
            x404_targetPosition = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
            intercept = x37c_projectileInfo.PredictInterceptPos(
                blastPos, mgr.GetPlayer()->GetAimPosition(mgr, 0.f), *mgr.GetPlayer(), false, dt);
          }

          const CVector3f targetDelta = x404_targetPosition - gun->GetTranslation();
          const CVector3f error = gun->GetTransform().Rotate(
              x3a4_burstFire.GetDistanceCompensatedError(targetDelta.Magnitude(), 20.f));
          gun->SetTargetPosition(x404_targetPosition + (intercept - x404_targetPosition + error));
        }
      }

      CVector3f targetDelta = x404_targetPosition - GetTranslation();
      targetDelta.SetZ(0.f);
      if (targetDelta.CanBeNormalized()) {
        const CVector3f targetDir = targetDelta.AsNormalized();
        const CVector3f front = GetTransform().GetForward();
        const float rotationAngle =
            rstl::min_val(dt * x2d4_data.GetTurnSpeed(), CVector3f::GetAngleDiff(targetDir, front));
        const CQuaternion rot = CQuaternion::LookAt(CUnitVector3f(front, CUnitVector3f::kN_No),
                                                    CUnitVector3f(targetDir, CUnitVector3f::kN_No),
                                                    CRelAngle::FromRadians(rotationAngle));
        const CQuaternion localRot = CQuaternion::ScalarVector(
            rot.GetScalar(), GetTransform().TransposeRotate(rot.GetVector()));
        RotateInOneFrameOR(localRot, dt);
      }

      if (ShouldFire(mgr)) {
        SendScriptMsgs(kSS_Attack, mgr, kSM_None);
        x560_26_firedWithSetBurst = true;
      }
      x52c_curActiveTime = 0.f;
    } else {
      x52c_curActiveTime += dt;
      if (x52c_curActiveTime >= 10.f) {
        SetTurretState(kTS_ExitTargeting, mgr);
      }
    }
    break;
  case kStateMsg_Deactivate:
    x560_30_needsStopClankSound = true;
    break;
  }
}

void CScriptGunTurret::ProcessExitTargettingState(EStateMsg msg, float dt, CStateManager& mgr) {
  switch (msg) {
  case kStateMsg_Update:
    if (x25c_gunId == kInvalidUniqueId) {
      break;
    }
    if (CScriptGunTurret* gun = TCastToPtr< CScriptGunTurret >(mgr.ObjectById(x25c_gunId))) {
      CTransform4f gunXf = GetTransform() * GetLocatorTransform(rstl::string_l(skGunLCTRName));
      const CVector3f front = gun->GetTransform().GetForward();
      const CVector3f originalFront = x544_originalFrontVec;
      if (CVector3f::GetAngleDiff(front, originalFront) < (M_PIF / 180.f)) {
        SetTurretState(kTS_Ready, mgr);
      }
    }
    break;
  default:
    break;
  }
}

void CScriptGunTurret::ProcessFrenzyState(EStateMsg msg, float dt, CStateManager& mgr) {
  switch (msg) {
  case kStateMsg_Activate:
    x560_31_frenzyReverse = mgr.Random()->Float() < 0.5f;
    x534_fireCycleRemTime = 0.15f;
    RemoveMaterial(kMT_Target, kMT_Orbit, mgr);
    mgr.Player()->TryToBreakOrbit(GetUniqueId(), CPlayer::kOB_ActivateOrbitSource, mgr);
    break;
  case kStateMsg_Update: {
    if (x524_curStateTime >= x2d4_data.GetFrenzyDuration()) {
      SetTurretState(kTS_Destroyed, mgr);
      if (CScriptGunTurret* gun = TCastToPtr< CScriptGunTurret >(mgr.ObjectById(x25c_gunId))) {
        gun->x520_state = kTS_Destroyed;
      }
      break;
    }

    const CVector3f frontVec = GetTransform().GetForward();
    if (x560_31_frenzyReverse) {
      if (CVector3f::Dot(x550_originalRightVec, frontVec) < 0.f &&
          CVector3f::GetAngleDiff(x544_originalFrontVec, frontVec) >= CMath::Deg2Rad(45.f)) {
        x560_31_frenzyReverse = false;
      }
    } else {
      if (CVector3f::Dot(x550_originalRightVec, frontVec) > 0.f &&
          CVector3f::GetAngleDiff(x544_originalFrontVec, frontVec) >= CMath::Deg2Rad(45.f)) {
        x560_31_frenzyReverse = true;
      }
    }

    if (CScriptGunTurret* gun = TCastToPtr< CScriptGunTurret >(mgr.ObjectById(x25c_gunId))) {
      x534_fireCycleRemTime -= dt;
      if (x534_fireCycleRemTime < 0.f) {
        x404_targetPosition = gun->GetTranslation() + 100.f * gun->GetTransform().GetForward();
        SendScriptMsgs(kSS_Attack, mgr, kSM_None);
        x534_fireCycleRemTime = 0.15f;
      }
    }
    break;
  }
  default:
    break;
  }
}

void CScriptGunTurret::UpdateTurretAnimation() {
  if (HasAnimation() && x520_state >= kTS_Destroyed && x520_state <= kTS_Frenzy) {
    const CPASAnimParmData parms(pas::kAS_Locomotion, CPASAnimParm::FromEnum(0),
                                 CPASAnimParm::FromEnum(skStateToLocoTypeLookup[x520_state]));
    const rstl::pair< float, int > best =
        GetAnimationData()->GetPASDatabase().FindBestAnimation(parms, -1);
    if (best.first > 0.f && best.second != x540_turretAnim) {
      AnimationData()->SetAnimation(CAnimPlaybackParms(best.second, -1, 1.f, true), false);
      ModelData()->EnableLooping(true);
      x540_turretAnim = best.second;
    }
  }
}

void CScriptGunTurret::UpdateTargettingMode(float dt, CStateManager& mgr) {
  if (mgr.GetCameraManager()->IsInCinematicCamera()) {
    x534_fireCycleRemTime =
        mgr.Random()->Float() * x2d4_data.GetReloadTimeVariance() + x2d4_data.GetReloadTime();
    x538_halfFireCycleDur = 0.5f * x534_fireCycleRemTime;
  }
  if (x534_fireCycleRemTime > 0.f) {
    x534_fireCycleRemTime -= dt;
    if (x534_fireCycleRemTime < x538_halfFireCycleDur && x520_state != kTS_Firing) {
      CSfxManager::AddEmitter(x2d4_data.GetChargingSoundId(), GetTranslation(), CVector3f::Up(),
                              false, false, CSfxManager::kMedPriority, GetCurrentAreaId().Value());
      SetTurretState(kTS_Firing, mgr);
    }
  } else {
    if (x520_state != kTS_Targeting) {
      SetTurretState(kTS_Targeting, mgr);
    }
    if (!x3a4_burstFire.IsBurstSet()) {
      UpdateBurstType(mgr);
      x534_fireCycleRemTime =
          mgr.Random()->Float() * x2d4_data.GetReloadTimeVariance() + x2d4_data.GetReloadTime();
      x538_halfFireCycleDur = 0.5f * x534_fireCycleRemTime;
    } else {
      x3a4_burstFire.Update(mgr, dt);
    }
  }
}

void CScriptGunTurret::UpdateGunOrientation(float dt, CStateManager& mgr) {
  if (x25c_gunId != kInvalidUniqueId) {
    if (CScriptGunTurret* gun = TCastToPtr< CScriptGunTurret >(mgr.ObjectById(x25c_gunId))) {
      const CTransform4f gunXf =
          GetTransform() * GetLocatorTransform(rstl::string_l(skGunLCTRName));
      switch (x520_state) {
      case kTS_Targeting:
      case kTS_Firing: {
        const CVector3f gunPos = gunXf.GetTranslation();
        float yaw = 0.f;
        const float horizontalMag =
            sqrt(gunXf.Get11() * gunXf.Get11() + gunXf.Get01() * gunXf.Get01());
        if (horizontalMag > 0.001f) {
          const double angle = -atan2(gunXf.Get01(), gunXf.Get11());
          yaw = angle;
        }
        const float oldPitch = gun->GetPitch();
        float pitch = 0.f;
        if (!gun->PlayerInsideTurretSphere(mgr)) {
          const CVector3f targetDelta = x404_targetPosition - gunPos;
          const CTransform4f targetXf = targetDelta.CanBeNormalized()
                                            ? CTransform4f::LookAt(gunPos, x404_targetPosition)
                                            : GetTransform();
          const float targetHorizontalMag = CMath::SqrtF(targetXf.Get11() * targetXf.Get11() +
                                                         targetXf.Get01() * targetXf.Get01());
          const float newPitch = CCast::ToReal32(-atan2(-targetXf.Get21(), targetHorizontalMag));
          const float delta = newPitch - oldPitch;
          const float step =
              delta > 0.f ? dt * x2d4_data.GetTurnSpeed() : dt * -x2d4_data.GetTurnSpeed();
          pitch = fabsf(delta) <= fabsf(step) ? newPitch : oldPitch + step;
          pitch = rstl::max_val(pitch, -x2d4_data.GetDownMaxAngle());
        }
        const CQuaternion rotation =
            CQuaternion::YXZRotation(CRelAngle::FromRadians(0.f), CRelAngle::FromRadians(pitch),
                                     CRelAngle::FromRadians(yaw));
        gun->SetTransform(CTransform4f(rotation.BuildTransform(), gunPos));
        break;
      }
      case kTS_ExitTargeting: {
        const float angle = 0.3f * (dt * x2d4_data.GetTurnSpeed());
        const CVector3f front = GetTransform().GetForward();
        const CVector3f gunFront = gun->GetTransform().GetForward();
        const CQuaternion gunRotation = CQuaternion::LookAt(
            CUnitVector3f(gunFront.GetX(), gunFront.GetY(), gunFront.GetZ()),
            CUnitVector3f(front.GetX(), front.GetY(), front.GetZ()), CRelAngle::FromRadians(angle));
        const CQuaternion localGunRotation = CQuaternion::ScalarVector(
            gunRotation.GetScalar(), gun->GetTransform().TransposeRotate(gunRotation.GetVector()));
        gun->RotateInOneFrameOR(localGunRotation, dt);
        const CVector3f originalFront = x544_originalFrontVec;
        const CVector3f currentFront = GetTransform().GetForward();
        const CQuaternion rotation = CQuaternion::LookAt(
            CUnitVector3f(currentFront.GetX(), currentFront.GetY(), currentFront.GetZ()),
            CUnitVector3f(originalFront.GetX(), originalFront.GetY(), originalFront.GetZ()),
            CRelAngle::FromRadians(angle));
        const CQuaternion localRotation = CQuaternion::ScalarVector(
            rotation.GetScalar(), GetTransform().TransposeRotate(rotation.GetVector()));
        RotateInOneFrameOR(localRotation, dt);
        break;
      }
      case kTS_Frenzy: {
        const CVector3f gunPos = gunXf.GetTranslation();
        float yaw = 0.f;
        const float horizontalMag =
            sqrt(gunXf.Get11() * gunXf.Get11() + gunXf.Get01() * gunXf.Get01());
        if (horizontalMag > 0.001f) {
          const double angle = -atan2(gunXf.Get01(), gunXf.Get11());
          yaw = angle;
        }
        const float turnSpeed = x2d4_data.GetTurnSpeed();
        const float newPitch = -0.5f * x2d4_data.GetDownMaxAngle() *
                               (1.f - CMath::FastCosR(2.f * x524_curStateTime * turnSpeed));
        const float oldPitch = gun->GetPitch();
        const float delta = newPitch - oldPitch;
        const float angle = turnSpeed * dt;
        const float step = delta > 0.f ? angle : -angle;
        float pitch = fabsf(delta) <= fabsf(step) ? newPitch : oldPitch + step;
        pitch = rstl::max_val(pitch, -x2d4_data.GetDownMaxAngle());
        const CQuaternion gunRotation =
            CQuaternion::YXZRotation(CRelAngle::FromRadians(0.f), CRelAngle::FromRadians(pitch),
                                     CRelAngle::FromRadians(yaw));
        gun->SetTransform(CTransform4f(gunRotation.BuildTransform(), gunPos));
        const CVector3f targetDir =
            x560_31_frenzyReverse ? -x550_originalRightVec : x550_originalRightVec;
        const CVector3f currentFront = GetTransform().GetForward();
        const CQuaternion rotation = CQuaternion::LookAt(
            CUnitVector3f(currentFront.GetX(), currentFront.GetY(), currentFront.GetZ()),
            CUnitVector3f(targetDir.GetX(), targetDir.GetY(), targetDir.GetZ()),
            CRelAngle::FromRadians(angle));
        const CQuaternion localRotation = CQuaternion::ScalarVector(
            rotation.GetScalar(), GetTransform().TransposeRotate(rotation.GetVector()));
        RotateInOneFrameOR(localRotation, dt);
        break;
      }
      default:
        gun->SetTransform(gunXf);
        break;
      }
    }
  }
}

CVector3f CScriptGunTurret::UpdateExtensionModelState(float dt) {
  CVector3f offset = CVector3f::Zero();
  if (x4a4_extensionModel) {
    const float change = 1.5f * dt;
    switch (x520_state) {
    case kTS_PanningA:
    case kTS_PanningB:
    case kTS_Targeting:
    case kTS_Firing:
    case kTS_ExitTargeting:
      x4f8_extensionT = rstl::min_val(x4f8_extensionT + change, skExtensionOverlapMaxPer);
      break;
    case kTS_Ready:
    case kTS_Deactivating:
    case kTS_DeactivatingFromReady:
    case kTS_Frenzy:
      break;
    default:
      x4f8_extensionT = rstl::max_val(0.f, x4f8_extensionT - change);
      break;
    }
    offset = x4fc_extensionOffset +
             x2d4_data.GetExtensionDropDownDist() * (x4f8_extensionT * CVector3f::Down()) -
             GetTranslation();
  }
  return offset;
}

void CScriptGunTurret::UpdateHealthInfo(CStateManager& mgr) {
  switch (x258_type) {
  case kTC_Base:
    if (x25c_gunId != kInvalidUniqueId) {
      if (!TCastToPtr< CScriptGunTurret >(mgr.ObjectById(x25c_gunId))) {
        SetTurretState(kTS_Destroyed, mgr);
        x560_25_frozen = false;
        x25c_gunId = kInvalidUniqueId;
        if (x50c_targetingEmitter) {
          CSfxManager::RemoveEmitter(x50c_targetingEmitter);
          x50c_targetingEmitter.Clear();
        }
      }
    } else {
      SetTurretState(kTS_Destroyed, mgr);
    }
    break;
  case kTC_Gun:
    if (!x560_24_dead && x520_state != kTS_Frenzy && HealthInfo(mgr)->GetHP() <= 0.f) {
      x560_24_dead = true;
      SendScriptMsgs(kSS_Dead, mgr, kSM_None);
      mgr.DeleteObjectRequest(GetUniqueId());
    }
    break;
  default:
    break;
  }
}

bool CScriptGunTurret::IsStopped(const float dt) const {
  const CVector2f thisForward(GetTransform().GetForward().ToVec2f());
  const CVector2f lastForward(x514_lastFrontVector.ToVec2f());

  return CVector2f::GetAngleDiff(lastForward, thisForward) < dt * (2 * (M_PIF / 180));
}

bool CScriptGunTurret::IsPlayerInFiringRange(CStateManager& mgr) const {
  const CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
  const CVector3f horizontalDelta(delta.ToVec2f(), 0.f);
  bool inHorizontalRange;
  if (CVector3f::Dot(x550_originalRightVec, delta) >= 0.f) {
    inHorizontalRange = CVector3f::GetAngleDiff(x544_originalFrontVec, horizontalDelta) <=
                        x2d4_data.GetRightMaxAngle();
  } else {
    inHorizontalRange = CVector3f::GetAngleDiff(x544_originalFrontVec, horizontalDelta) <=
                        x2d4_data.GetLeftMaxAngle();
  }
  if (inHorizontalRange) {
    const float pitch = CVector3f::GetAngleDiff(CVector3f::Up(), delta) - M_PIF / 2.f;
    inHorizontalRange = pitch >= -20.f * (M_PIF / 180.f) && pitch <= x2d4_data.GetDownMaxAngle();
  }
  return inHorizontalRange;
}

bool CScriptGunTurret::InDetectionRange(CStateManager& mgr) const {
  const CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
  if ((CVector3f::Dot(delta, CVector3f::Down()) >= 0.f ||
       CVector3f::GetAngleDiff(GetTransform().GetForward(), delta) <= 20.f * (M_PIF / 180.f)) &&
      delta.MagSquared() <= x2d4_data.GetDetectionRange() * x2d4_data.GetDetectionRange() &&
      (x2d4_data.GetDetectionZRange() == 0.f ||
       CMath::AbsF(delta.GetZ()) < x2d4_data.GetDetectionZRange())) {
    return LineOfSightTest(mgr);
  }
  return false;
}

bool CScriptGunTurret::ShouldFire(CStateManager& mgr) const {
  if (x520_state == kTS_Targeting && x534_fireCycleRemTime <= 0.f && x3a4_burstFire.ShouldFire()) {
    return IsPlayerInFiringRange(mgr);
  }
  return false;
}

void CScriptGunTurret::LaunchProjectile(CStateManager& mgr) {
  if (x37c_projectileInfo.Token().TryCache() && mgr.CanCreateProjectile(GetUniqueId(), kWT_AI, 8)) {
    const CTransform4f blastXf = GetLocatorTransform(rstl::string_l(skBlastLCTRName));
    const CVector3f projectilePos =
        GetTranslation() + GetTransform().Rotate(blastXf.GetTranslation());
    CVector3f lookPos = x404_targetPosition;
    const CVector3f front = GetTransform().GetForward();
    const CVector3f delta = x404_targetPosition - projectilePos;
    if (CVector3f::GetAngleDiff(front, delta) > 20.f * (M_PIF / 180.f)) {
      if (delta.CanBeNormalized()) {
        const CVector3f dir =
            CVector3f::Slerp(front, delta.AsNormalized(), CRelAngle(20.f * (M_PIF / 180.f)));
        lookPos = projectilePos + delta.Magnitude() * dir;
      } else {
        lookPos = projectilePos + front;
      }
    } else if (!delta.CanBeNormalized()) {
      lookPos = projectilePos + front;
    }
    const CTransform4f projectileXf = CTransform4f::LookAt(projectilePos, lookPos);
    CEnergyProjectile* projectile = rs_new CEnergyProjectile(
        true, x37c_projectileInfo.Token(), kWT_AI, projectileXf, kMT_Character,
        x37c_projectileInfo.GetDamage(), mgr.AllocateUniqueId(), GetCurrentAreaId(), GetUniqueId(),
        kInvalidUniqueId, 0, false, CVector3f(1.f, 1.f, 1.f), x458_visorEffectDesc,
        x2d4_data.GetVisorSoundId(), false);
    if (projectile) {
      mgr.AddObject(projectile);
      const CPASAnimParmData parms(pas::kAS_ProjectileAttack, CPASAnimParm::FromEnum(1),
                                   CPASAnimParm::FromReal32(90.f),
                                   CPASAnimParm::FromEnum(skStateToLocoTypeLookup[x520_state]));
      const rstl::pair< float, int > anim =
          GetAnimationData()->GetCharacterInfo().GetPASDatabase().FindBestAnimation(parms, -1);
      if (anim.first > 0.f) {
        ModelData()->EnableLooping(false);
        AnimationData()->SetAnimation(CAnimPlaybackParms(anim.second, -1, 1.f, true), false);
      }
    }
  }
}

void CScriptGunTurret::PlayAdditiveFlinchAnimation(CStateManager& mgr) {
  const CPASAnimParmData parms(pas::kAS_AdditiveFlinch);
  const rstl::pair< float, int > best =
      GetAnimationData()->GetPASDatabase().FindBestAnimation(parms, *mgr.Random(), -1);
  if (best.first > 0.f) {
    AnimationData()->AddAdditiveAnimation(best.second, 1.f, false, true);
  }
}

void CScriptGunTurret::PlayAdditiveChargingAnimation(CStateManager& mgr) {
  if (x520_state == kTS_Firing) {
    if (x55c_additiveChargeAnim == -1) {
      const CPASAnimParmData parms(pas::kAS_AdditiveReaction, CPASAnimParm::FromEnum(2));
      CAnimData* animData = AnimationData();
      const rstl::pair< float, int > best =
          animData->GetPASDatabase().FindBestAnimation(parms, *mgr.Random(), -1);
      if (best.first > 0.f) {
        x55c_additiveChargeAnim = best.second;
        animData->AddAdditiveAnimation(x55c_additiveChargeAnim, 1.f, true, false);
      }
    }
  } else if (x55c_additiveChargeAnim != -1) {
    AnimationData()->DelAdditiveAnimation(x55c_additiveChargeAnim);
    x55c_additiveChargeAnim = -1;
  }
}

void CScriptGunTurret::UpdateGunParticles(float dt, CStateManager& mgr) {
  CGameLight* light = nullptr;
  if (x498_lightId != kInvalidUniqueId) {
    light = TCastToPtr< CGameLight >(mgr.ObjectById(x498_lightId));
  }
  if (!x560_25_frozen) {
    const CTransform4f lightXf = GetLocatorTransform(rstl::string_l(skLightLCTRName));
    const CVector3f lightPos = GetTranslation() + GetTransform().Rotate(lightXf.GetTranslation());
    if (light) {
      light->SetActive(true);
    }
    switch (x520_state) {
    case kTS_Deactivate:
    case kTS_DeactivateFromReady:
    case kTS_Deactivating:
    case kTS_DeactivatingFromReady:
      x468_idleLight->SetParticleEmission(false);
      x470_deactivateLight->SetParticleEmission(true);
      x478_targettingLight->SetParticleEmission(false);
      x480_frozenEffect->SetParticleEmission(false);
      x488_chargingEffect->SetParticleEmission(false);
      x490_panningEffect->SetParticleEmission(false);
      x470_deactivateLight->SetOrientation(GetTransform().GetRotation());
      x470_deactivateLight->SetGlobalTranslation(lightPos);
      x470_deactivateLight->SetGlobalScale(GetModelScale());
      x470_deactivateLight->Update(dt);
      if (light) {
        if (x470_deactivateLight->SystemHasLight()) {
          light->SetLight(x470_deactivateLight->GetLight());
        } else {
          light->SetActive(false);
        }
      }
      break;
    case kTS_Inactive:
      x468_idleLight->SetParticleEmission(true);
      x470_deactivateLight->SetParticleEmission(false);
      x478_targettingLight->SetParticleEmission(false);
      x480_frozenEffect->SetParticleEmission(false);
      x488_chargingEffect->SetParticleEmission(false);
      x490_panningEffect->SetParticleEmission(false);
      x468_idleLight->SetOrientation(GetTransform().GetRotation());
      x468_idleLight->SetGlobalTranslation(lightPos);
      x468_idleLight->SetGlobalScale(GetModelScale());
      x468_idleLight->Update(dt);
      if (light) {
        light->SetActive(false);
      }
      break;
    case kTS_PanningA:
    case kTS_PanningB:
      x468_idleLight->SetParticleEmission(false);
      x470_deactivateLight->SetParticleEmission(false);
      x478_targettingLight->SetParticleEmission(false);
      x480_frozenEffect->SetParticleEmission(false);
      x488_chargingEffect->SetParticleEmission(false);
      x490_panningEffect->SetParticleEmission(true);
      x490_panningEffect->SetOrientation(GetTransform().GetRotation());
      x490_panningEffect->SetGlobalTranslation(lightPos);
      x490_panningEffect->SetGlobalScale(GetModelScale());
      x490_panningEffect->Update(dt);
      if (light) {
        light->SetActive(false);
      }
      break;
    case kTS_Ready:
    case kTS_Targeting:
    case kTS_Firing:
    case kTS_ExitTargeting:
    case kTS_Frenzy: {
      const bool charging = x520_state == kTS_Firing || x520_state == kTS_Frenzy;
      x468_idleLight->SetParticleEmission(false);
      x470_deactivateLight->SetParticleEmission(false);
      x478_targettingLight->SetParticleEmission(true);
      x480_frozenEffect->SetParticleEmission(false);
      x488_chargingEffect->SetParticleEmission(charging);
      x478_targettingLight->SetOrientation(GetTransform().GetRotation());
      x478_targettingLight->SetGlobalTranslation(lightPos);
      x478_targettingLight->SetGlobalScale(GetModelScale());
      x478_targettingLight->Update(dt);
      if (x478_targettingLight->SystemHasLight()) {
        light->SetLight(x478_targettingLight->GetLight());
      } else {
        light->SetActive(false);
      }
      if (charging) {
        const CTransform4f blastXf = GetLocatorTransform(rstl::string_l(skBlastLCTRName));
        const CVector3f blastPos =
            GetTranslation() + GetTransform().Rotate(blastXf.GetTranslation());
        x488_chargingEffect->SetOrientation(GetTransform().GetRotation());
        x488_chargingEffect->SetGlobalTranslation(blastPos);
        x488_chargingEffect->SetGlobalScale(GetModelScale());
        x488_chargingEffect->Update(dt);
      }
      break;
    }
    case kTS_Destroyed:
    default:
      x468_idleLight->SetParticleEmission(false);
      x470_deactivateLight->SetParticleEmission(false);
      x478_targettingLight->SetParticleEmission(false);
      x480_frozenEffect->SetParticleEmission(false);
      x488_chargingEffect->SetParticleEmission(false);
      x490_panningEffect->SetParticleEmission(false);
      x490_panningEffect->Update(dt);
      if (light) {
        light->SetActive(false);
      }
      break;
    }
  } else {
    x468_idleLight->SetParticleEmission(false);
    x470_deactivateLight->SetParticleEmission(false);
    x478_targettingLight->SetParticleEmission(false);
    x480_frozenEffect->SetParticleEmission(true);
    x488_chargingEffect->SetParticleEmission(false);
    x490_panningEffect->SetParticleEmission(false);
    x480_frozenEffect->SetOrientation(GetTransform().GetRotation());
    x480_frozenEffect->SetGlobalTranslation(GetTranslation());
    x480_frozenEffect->SetGlobalScale(GetModelScale());
    x480_frozenEffect->Update(dt);
    if (light) {
      light->SetActive(false);
    }
  }
}

void CScriptGunTurret::UpdateBurstType(CStateManager& mgr) {
  if (x560_27_burstSet) {
    const bool inView = mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Morphed ||
                        CVector3f::Dot(GetTransform().GetForward(),
                                       mgr.GetPlayer()->GetTransform().GetForward()) < 0.f;
    const int viewOffset = inView ? 0 : 3;
    const int shots = mgr.Random()->Range(0, 3) + 2;
    int type;
    if (shots <= 2 || static_cast< int >(x2d4_data.GetNumSubsequentShots()) < 3) {
      type = 0;
    } else if (shots >= 5 && static_cast< int >(x2d4_data.GetNumSubsequentShots()) > 3) {
      type = 2;
    } else {
      type = 1;
    }
    x3a4_burstFire.SetBurstType(type + viewOffset);
  } else {
    x3a4_burstFire.SetBurstType(x2d4_data.GetNumInitialShots() - 2);
    x3a4_burstFire.SetFirstBurstIndex(x2d4_data.GetInitialShotTableIndex());
  }
  x3a4_burstFire.Start(mgr);
  x560_26_firedWithSetBurst = false;
  x560_27_burstSet = true;
}

void CScriptGunTurret::UpdateTargettingSound(float dt) {
  x510_timeSinceLastTargetSfx += dt;
  const CVector3f& front = GetTransform().GetForward();
  const CVector2f front2d = front.ToVec2f();
  const CVector2f lastFront2d = x514_lastFrontVector.ToVec2f();
  const float angle = CVector2f::GetAngleDiff(lastFront2d, front2d);
  if (x560_30_needsStopClankSound && angle < 20.f * (M_PIF / 180.f) &&
      (x520_state == kTS_Targeting || x520_state == kTS_Firing)) {
    if (!x560_25_frozen) {
      CSfxManager::AddEmitter(x2d4_data.GetStopClankSoundId(), GetTranslation(), CVector3f::Up(),
                              false, false, CSfxManager::kMedPriority, GetCurrentAreaId().Value());
    }
    x560_30_needsStopClankSound = false;
  }
  if (x510_timeSinceLastTargetSfx >= 0.5f && !x560_25_frozen) {
    if (x520_state == kTS_Targeting || x520_state == kTS_Firing || x520_state == kTS_Frenzy) {
      const bool stopped = IsStopped(dt);
      if (!stopped && !x50c_targetingEmitter) {
        x50c_targetingEmitter = CSfxManager::AddEmitter(
            x2d4_data.GetTrackingSoundId(), GetTranslation(), CVector3f::Zero(), false, true,
            CSfxManager::kMedPriority, GetCurrentAreaId().Value());
        x510_timeSinceLastTargetSfx = 0.f;
      } else if (stopped && x50c_targetingEmitter) {
        CSfxManager::RemoveEmitter(x50c_targetingEmitter);
        x50c_targetingEmitter.Clear();
        x510_timeSinceLastTargetSfx = 0.f;
      }
      if (x50c_targetingEmitter) {
        const float maxAngle = dt * x2d4_data.GetTurnSpeed();
        const float ratio = maxAngle > 0.f ? angle / maxAngle : 0.f;
        const float pitch = rstl::min_val(8192.f * ratio, 1.f);
        CSfxManager::PitchBend(x50c_targetingEmitter, 0x2000 + static_cast< int >(pitch));
      }
    } else if (x50c_targetingEmitter) {
      CSfxManager::RemoveEmitter(x50c_targetingEmitter);
      x50c_targetingEmitter.Clear();
      x510_timeSinceLastTargetSfx = 0.f;
    }
  } else if (x560_25_frozen && x50c_targetingEmitter) {
    CSfxManager::RemoveEmitter(x50c_targetingEmitter);
    x50c_targetingEmitter.Clear();
  }
  x514_lastFrontVector = GetTransform().GetForward();
}

bool CScriptGunTurret::LineOfSightTest(CStateManager& mgr) const {
  if (x25c_gunId != kInvalidUniqueId) {
    if (const CScriptGunTurret* const gun =
            TCastToPtr< CScriptGunTurret >(mgr.ObjectById(x25c_gunId))) {
      if (x560_27_burstSet || (x520_state == kTS_Inactive && x4a4_extensionModel)) {
        return true;
      }
      const CTransform4f blastXf = gun->GetLocatorTransform(rstl::string_l(skBlastLCTRName));
      const CVector3f muzzlePos =
          gun->GetTranslation() + gun->GetTransform().Rotate(blastXf.GetTranslation());
      CVector3f dir = mgr.GetPlayer()->GetAimPosition(mgr, 0.f) - muzzlePos;
      const float distance = dir.Magnitude();
      dir /= distance;
      TEntityList nearList;
      const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
          CMaterialList(kMT_Solid), CMaterialList(kMT_Player, kMT_CollisionActor));
      mgr.BuildNearList(nearList, muzzlePos, dir, distance, filter, gun);
      TUniqueId id = kInvalidUniqueId;
      return mgr.RayWorldIntersection(id, muzzlePos, dir, distance, filter, nearList).IsInvalid();
    }
  }
  return false;
}

void CScriptGunTurret::SetupCollisionManager(CStateManager& mgr) {
  rstl::vector< CJointCollisionDescription > jointDescs;
  jointDescs.reserve(2);
  const CAnimData* animData = GetAnimationData();
  x508_gunSDKSeg = animData->GetLocatorSegId(rstl::string_l(skGunLCTRName));
  const CSegId blastSeg = animData->GetLocatorSegId(rstl::string_l(skBlastLCTRName));
  const CJointCollisionDescription gunDesc = CJointCollisionDescription::SphereSubdivideCollision(
      x508_gunSDKSeg, blastSeg, 0.6f, 1.f, CJointCollisionDescription::kOT_One,
      rstl::string_l(skGunLCTRName), 1000.f);
  jointDescs.push_back(gunDesc);
  const CJointCollisionDescription blastDesc = CJointCollisionDescription::SphereCollision(
      blastSeg, 0.3f, rstl::string_l(skBlastLCTRName), 1000.f);
  jointDescs.push_back(blastDesc);
  x49c_collisionManager =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), jointDescs, true);
  x49c_collisionManager->SetActive(mgr, GetActive());
  for (uint i = 0; i < x49c_collisionManager->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& desc = x49c_collisionManager->GetCollisionDescFromIndex(i);
    const TUniqueId actorId = desc.GetCollisionActorId();
    if (CCollisionActor* const actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(actorId))) {
      actor->AddMaterial(kMT_ProjectilePassthrough, mgr);
      actor->SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
          CMaterialList(kMT_Player),
          CMaterialList(kMT_Character, kMT_NoStaticCollision, kMT_NoPlatformCollision)));
      if (desc.GetName() == rstl::string_l(skBlastLCTRName)) {
        x4a0_collisionActor = actorId;
      }
    }
  }
}

void CScriptGunTurret::UpdateGunCollisionManager(float dt, CStateManager& mgr) {
  if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(x4a0_collisionActor))) {
    actor->SetActive(mgr.GetPlayer()->GetMorphballTransitionState() != CPlayer::kMS_Morphed);
  }
  x49c_collisionManager->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
}

void CScriptGunTurret::UpdateFrozenState(float dt, CStateManager& mgr) {
  if (x560_25_frozen) {
    if (x53c_freezeRemTime <= 0.f) {
      x560_25_frozen = false;
      SendScriptMsgs(kSS_UnFrozen, mgr, kSM_None);
      CSfxManager::AddEmitter(x2d4_data.GetUnFreezeSoundId(), GetTranslation(), CVector3f::Up(),
                              false, false, CSfxManager::kMedPriority, GetCurrentAreaId().Value());
      SetMuted(false);
    } else if (x2d4_data.UseFreezeTimeout()) {
      x53c_freezeRemTime -= dt;
    }
  } else {
    x53c_freezeRemTime = 0.f;
  }
}

bool CScriptGunTurret::PlayerInsideTurretSphere(CStateManager& mgr) {
  if (const CCollisionActor* actor =
          TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(x4a0_collisionActor))) {
    if (actor->GetActive()) {
      const CVector3f aimPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
      const CVector3f delta = aimPos - GetTranslation();
      if (delta.GetZ() < 0.f) {
        const CVector3f actorDelta = actor->GetTranslation() - GetTranslation();
        const float radius = actor->GetSphereRadius() * 2.f + actorDelta.Magnitude();
        return delta.MagSquared() < radius * radius;
      }
    }
  }
  return false;
}
