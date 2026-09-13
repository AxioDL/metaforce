#include "MetroidPrime/ScriptObjects/CScriptPickup.hpp"

#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CAnimPlaybackParms.hpp"
#include "MetroidPrime/CArtifactDoll.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CModelData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerGun.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/Tweaks/CTweakGame.hpp"

#include "MetroidPrime/HUD/CHUDMemoParms.hpp"
#include "MetroidPrime/HUD/CSamusHud.hpp"

#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Math/CAbsAngle.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Text/CStringTable.hpp"

#include "rstl/math.hpp"

static float skDrawInDistance = 30.f;

CScriptPickup::CScriptPickup(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                             const CTransform4f& xf, const CModelData& mData,
                             const CActorParameters& aParams, const CAABox& aabb,
                             CPlayerState::EItemType itemType, int amount, int capacity,
                             CAssetId pickupEffect, float possibility, float lifeTime,
                             float fadeInTime, float startDelay, bool active)
: CPhysicsActor(uid, active, name, info, xf, mData, CMaterialList(), aabb,
                SMoverData(1.f, CVector3f::Zero(), CAxisAngle::Identity(), CVector3f::Zero(),
                           CAxisAngle::Identity()),
                aParams, 0.3f, 0.1f)
, x258_itemType(itemType)
, x25c_amount(amount)
, x260_capacity(capacity)
, x264_possibility(possibility)
, x268_fadeInTime(fadeInTime)
, x26c_lifeTime(lifeTime)
, x270_curTime(0.f)
, x278_delayTimer(startDelay)
, x28c_24_generated(false)
, x28c_25_inTractor(false)
, x28c_26_enableTractorTest(false) {
  if (pickupEffect != kInvalidAssetId) {
    x27c_pickupParticleDesc = gpSimplePool->GetObj(SObjectTag('PART', pickupEffect));
    x27c_pickupParticleDesc->Lock();
  }

  if (HasAnimation()) {
    AnimationData()->SetAnimation(CAnimPlaybackParms(0, -1, 1.f, true), false);
  }

  if (x268_fadeInTime) {
    SetModelFlags(CModelFlags::AlphaBlended(0.f).DepthCompareUpdate(true, false));
  }
}

CScriptPickup::~CScriptPickup() {}

void CScriptPickup::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  if (x278_delayTimer >= 0.f) {
    CPhysicsActor::Stop();
    x278_delayTimer -= dt;
    return;
  }

  x270_curTime += dt;
  if (x28c_25_inTractor && (x26c_lifeTime - x270_curTime) < 2.f) {
    x270_curTime = rstl::max_val(x26c_lifeTime - 2.f - FLT_EPSILON, x270_curTime - 2.f * dt);
  }

  CModelFlags drawFlags = CModelFlags::Normal();

  if (x268_fadeInTime) {
    if (x270_curTime < x268_fadeInTime) {
      drawFlags =
          CModelFlags::AlphaBlended(x270_curTime / x268_fadeInTime).DepthCompareUpdate(true, false);
    } else {
      x268_fadeInTime = 0.f;
    }
  } else if (x26c_lifeTime) {
    float alpha = 1.f;
    if (x26c_lifeTime < 2.f) {
      alpha = 1.f - (x26c_lifeTime / x270_curTime);
    } else if ((x26c_lifeTime - x270_curTime) < 2.f) {
      alpha = (x26c_lifeTime - x270_curTime) / 2.f;
    }

    drawFlags = CModelFlags::AlphaBlended(alpha).DepthCompareUpdate(true, false);
  }

  SetModelFlags(drawFlags);

  if (HasAnimation()) {
    CAdvancementDeltas deltas = UpdateAnimation(dt, mgr, true);
    MoveToOR(deltas.GetOffsetDelta(), dt);
    RotateToOR(deltas.GetOrientationDelta(), dt);
  }

  if (x28c_25_inTractor) {
    CVector3f velocity =
        mgr.GetPlayer()->GetTranslation() + (CVector3f::Up() * 2.f) - GetTranslation();
    x274_tractorTime += dt;
    float halfTractorTime = rstl::min_val(x274_tractorTime, 2.f) * 0.5f;
    velocity = velocity.AsNormalized() * (halfTractorTime * 20.f);
    if (x28c_26_enableTractorTest && mgr.GetPlayer()->GetPlayerGun()->GetChargePercentage() <
                                         CPlayerGun::GetTractorBeamFactor()) {
      x28c_26_enableTractorTest = false;
      x28c_25_inTractor = false;
      velocity = CVector3f::Zero();
    }
    SetVelocityWR(velocity);
  } else if (x28c_24_generated) {
    if (mgr.GetPlayer()->GetPlayerGun()->GetChargePercentage() >
        CPlayerGun::GetTractorBeamFactor()) {
      const CFirstPersonCamera* camera = mgr.CameraManager()->FirstPersonCamera();
      CVector3f posDelta = GetTranslation() - camera->GetTranslation();
      CVector3f cameraFront = camera->GetTransform().GetColumn(kDY);
      float dot = CVector3f::Dot(cameraFront, posDelta.AsNormalized());
      float fovCos = cosine(CAbsAngle::FromDegrees(gpTweakGame->GetFirstPersonFOV()));
      if (dot > fovCos && posDelta.MagSquared() < skDrawInDistance * skDrawInDistance) {
        x28c_25_inTractor = true;
        x28c_26_enableTractorTest = true;
        x274_tractorTime = 0.f;
      }
    }
  }

  if (x26c_lifeTime && x270_curTime > x26c_lifeTime) {
    mgr.DeleteObjectRequest(GetUniqueId());
  }
}

void CScriptPickup::Touch(CActor& act, CStateManager& mgr) {
  if (GetActive() && !(x278_delayTimer >= 0) && TCastToPtr< CPlayer >(act)) {
    CPlayerState::EItemType itemType = x258_itemType;
    if (itemType >= CPlayerState::kIT_Truth && itemType <= CPlayerState::kIT_Newborn) {
      CAssetId id = CArtifactDoll::GetArtifactHeadScanFromItemType(itemType);
      if (id != kInvalidAssetId) {
        mgr.PlayerState()->SetScanTime(id, 0.5f);
      }
    }

    if (x27c_pickupParticleDesc) {
      if (mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::kPV_Thermal) {
        mgr.AddObject(rs_new CExplosion(
            TLockedToken< CGenDescription >(*x27c_pickupParticleDesc), mgr.AllocateUniqueId(), true,
            CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, kInvalidEditorId),
            rstl::string_l("Explosion - Pickup Effect"), GetTransform(), 0,
            CVector3f(1.f, 1.f, 1.f), CColor::White()));
      }
    }

    mgr.PlayerState()->InitializePowerUp(itemType, x260_capacity);
    mgr.PlayerState()->IncrPickUp(itemType, x25c_amount);
    mgr.DeleteObjectRequest(GetUniqueId());
    SendScriptMsgs(kSS_Arrived, mgr, kSM_None);

    if (x260_capacity > 0) {
      const CPlayerState* playerState = mgr.GetPlayerState();
      int total = playerState->GetTotalPickupCount();
      int colRate = playerState->CalculateItemCollectionRate();
      if (colRate == total) {
        CSystemState& state = gpGameState->SystemState();
        CAssetId id =
            gpResourceFactory
                ->GetResourceIdByName(state.GetAllItemsCollected() ? "STRG_AllPickupsFound_2"
                                                                   : "STRG_AllPickupsFound_1")
                ->GetId();
        mgr.QueueMessage(mgr.GetHUDMessageFrameCount() + 1, id, 0.f);
        state.SetAllItemsCollected(true);
      }
    }

    if (itemType == CPlayerState::kIT_PowerBombs) {
      CSystemState& state = gpGameState->SystemState();
      if (state.GetShowPowerBombAmmoMessage()) {
        state.IncrementPowerBombAmmoCount();
        CSamusHud::DisplayHudMemo(rstl::wstring_l(gpStringTable->GetString(109)),
                                  CHUDMemoParms(5.f, true, false, false));
      }
    }
  }
}

rstl::optional_object< CAABox > CScriptPickup::GetTouchBounds() const {
  return CPhysicsActor::GetBoundingBox();
}

void CScriptPickup::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPhysicsActor::AcceptScriptMsg(msg, uid, mgr);
}

void CScriptPickup::Render(const CStateManager& mgr) const { CPhysicsActor::Render(mgr); }

ENTITY_ACCEPT_IMPL(CScriptPickup)

CPlayerState::EItemType CScriptPickup::GetItem() const { return x258_itemType; }

float CScriptPickup::GetPossibility() const { return x264_possibility; }

void CScriptPickup::SetWasGenerated() { x28c_24_generated = true; }
