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
, mItemType(itemType)
, mAmount(amount)
, mCapacity(capacity)
, mPossibility(possibility)
, mFadeInTime(fadeInTime)
, mLifeTime(lifeTime)
, mCurTime(0.f)
, mDelayTimer(startDelay)
, mGenerated(false)
, mInTractor(false)
, mEnableTractorTest(false) {
  if (pickupEffect != kInvalidAssetId) {
    mPickupParticleDesc = gpSimplePool->GetObj(SObjectTag('PART', pickupEffect));
    mPickupParticleDesc->Lock();
  }

  if (HasAnimation()) {
    AnimationData()->SetAnimation(CAnimPlaybackParms(0, -1, 1.f, true), false);
  }

  if (mFadeInTime) {
    SetModelFlags(CModelFlags::AlphaBlended(0.f).DepthCompareUpdate(true, false));
  }
}

CScriptPickup::~CScriptPickup() {}

void CScriptPickup::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  if (mDelayTimer >= 0.f) {
    CPhysicsActor::Stop();
    mDelayTimer -= dt;
    return;
  }

  mCurTime += dt;
  if (mInTractor && (mLifeTime - mCurTime) < 2.f) {
    mCurTime = rstl::max_val(mLifeTime - 2.f - FLT_EPSILON, mCurTime - 2.f * dt);
  }

  CModelFlags drawFlags = CModelFlags::Normal();

  if (mFadeInTime) {
    if (mCurTime < mFadeInTime) {
      drawFlags =
          CModelFlags::AlphaBlended(mCurTime / mFadeInTime).DepthCompareUpdate(true, false);
    } else {
      mFadeInTime = 0.f;
    }
  } else if (mLifeTime) {
    float alpha = 1.f;
    if (mLifeTime < 2.f) {
      alpha = 1.f - (mLifeTime / mCurTime);
    } else if ((mLifeTime - mCurTime) < 2.f) {
      alpha = (mLifeTime - mCurTime) / 2.f;
    }

    drawFlags = CModelFlags::AlphaBlended(alpha).DepthCompareUpdate(true, false);
  }

  SetModelFlags(drawFlags);

  if (HasAnimation()) {
    CAdvancementDeltas deltas = UpdateAnimation(dt, mgr, true);
    MoveToOR(deltas.GetOffsetDelta(), dt);
    RotateToOR(deltas.GetOrientationDelta(), dt);
  }

  if (mInTractor) {
    CVector3f velocity =
        mgr.GetPlayer()->GetTranslation() + (CVector3f::Up() * 2.f) - GetTranslation();
    mTractorTime += dt;
    float halfTractorTime = rstl::min_val(mTractorTime, 2.f) * 0.5f;
    velocity = velocity.AsNormalized() * (halfTractorTime * 20.f);
    if (mEnableTractorTest && mgr.GetPlayer()->GetPlayerGun()->GetChargePercentage() <
                                         CPlayerGun::GetTractorBeamFactor()) {
      mEnableTractorTest = false;
      mInTractor = false;
      velocity = CVector3f::Zero();
    }
    SetVelocityWR(velocity);
  } else if (mGenerated) {
    if (mgr.GetPlayer()->GetPlayerGun()->GetChargePercentage() >
        CPlayerGun::GetTractorBeamFactor()) {
      const CFirstPersonCamera* camera = mgr.CameraManager()->FirstPersonCamera();
      CVector3f posDelta = GetTranslation() - camera->GetTranslation();
      CVector3f cameraFront = camera->GetTransform().GetColumn(kDY);
      float dot = CVector3f::Dot(cameraFront, posDelta.AsNormalized());
      float fovCos = cosine(CAbsAngle::FromDegrees(gpTweakGame->GetFirstPersonFOV()));
      if (dot > fovCos && posDelta.MagSquared() < skDrawInDistance * skDrawInDistance) {
        mInTractor = true;
        mEnableTractorTest = true;
        mTractorTime = 0.f;
      }
    }
  }

  if (mLifeTime && mCurTime > mLifeTime) {
    mgr.DeleteObjectRequest(GetUniqueId());
  }
}

void CScriptPickup::Touch(CActor& act, CStateManager& mgr) {
  if (GetActive() && !(mDelayTimer >= 0) && TCastToPtr< CPlayer >(act)) {
    CPlayerState::EItemType itemType = mItemType;
    if (itemType >= CPlayerState::kIT_Truth && itemType <= CPlayerState::kIT_Newborn) {
      CAssetId id = CArtifactDoll::GetArtifactHeadScanFromItemType(itemType);
      if (id != kInvalidAssetId) {
        mgr.PlayerState()->SetScanTime(id, 0.5f);
      }
    }

    if (mPickupParticleDesc) {
      if (mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::kPV_Thermal) {
        mgr.AddObject(rs_new CExplosion(
            TLockedToken< CGenDescription >(*mPickupParticleDesc), mgr.AllocateUniqueId(), true,
            CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList, kInvalidEditorId),
            rstl::string_l("Explosion - Pickup Effect"), GetTransform(), 0,
            CVector3f(1.f, 1.f, 1.f), CColor::White()));
      }
    }

    mgr.PlayerState()->InitializePowerUp(itemType, mCapacity);
    mgr.PlayerState()->IncrPickUp(itemType, mAmount);
    mgr.DeleteObjectRequest(GetUniqueId());
    SendScriptMsgs(kSS_Arrived, mgr, kSM_None);

    if (mCapacity > 0) {
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

CPlayerState::EItemType CScriptPickup::GetItem() const { return mItemType; }

float CScriptPickup::GetPossibility() const { return mPossibility; }

void CScriptPickup::SetWasGenerated() { mGenerated = true; }
