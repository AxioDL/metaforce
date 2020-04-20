#include "Runtime/World/CScriptPickup.hpp"

#include "Runtime/CGameOptions.hpp"
#include "Runtime/CGameState.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Camera/CFirstPersonCamera.hpp"
#include "Runtime/GuiSys/CStringTable.hpp"
#include "Runtime/MP1/CArtifactDoll.hpp"
#include "Runtime/MP1/CSamusHud.hpp"
#include "Runtime/Particle/CGenDescription.hpp"

#include "Runtime/World/CExplosion.hpp"
#include "Runtime/World/CPlayer.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {
CScriptPickup::CScriptPickup(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                             CModelData&& mData, const CActorParameters& aParams, const zeus::CAABox& aabb,
                             CPlayerState::EItemType itemType, s32 amount, s32 capacity, CAssetId pickupEffect,
                             float possibility, float lifeTime, float fadeInTime, float startDelay, bool active)
: CPhysicsActor(uid, active, name, info, xf, std::move(mData), CMaterialList(), aabb, SMoverData(1.f), aParams, 0.3f,
                0.1f)
, x258_itemType(itemType)
, x25c_amount(amount)
, x260_capacity(capacity)
, x264_possibility(possibility)
, x268_fadeInTime(fadeInTime)
, x26c_lifeTime(lifeTime)
, x278_delayTimer(startDelay) {
  if (pickupEffect.IsValid())
    x27c_pickupParticleDesc = g_SimplePool->GetObj({SBIG('PART'), pickupEffect});

  if (x64_modelData && x64_modelData->GetAnimationData())
    x64_modelData->GetAnimationData()->SetAnimation(CAnimPlaybackParms(0, -1, 1.f, true), false);

  if (x278_delayTimer != 0.f) {
    xb4_drawFlags = CModelFlags(5, 0, 3, zeus::CColor(1.f, 1.f, 1.f, 0.f));
    xb4_drawFlags.x2_flags &= 0xFFFC;
    xb4_drawFlags.x2_flags |= 1;
  }
}

void CScriptPickup::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptPickup::Think(float dt, CStateManager& mgr) {
  if (!GetActive())
    return;

  if (x278_delayTimer >= 0.f) {
    CPhysicsActor::Stop();
    x278_delayTimer -= dt;
    return;
  }

  x270_curTime += dt;
  if (x28c_25_inTractor && (x26c_lifeTime - x270_curTime) < 2.f)
    x270_curTime = zeus::max(x270_curTime - 2.f * dt, x26c_lifeTime - 2.f - FLT_EPSILON);

  CModelFlags drawFlags{0, 0, 3, zeus::CColor(1.f, 1.f, 1.f, 1.f)};

  if (x268_fadeInTime != 0.f) {
    if (x270_curTime < x268_fadeInTime) {
      drawFlags = CModelFlags(5, 0, 3, zeus::CColor(1.f, x270_curTime / x268_fadeInTime));
      drawFlags.x2_flags &= 0xFFFC;
      drawFlags.x2_flags |= 1;
    } else {
      x268_fadeInTime = 0.f;
    }
  } else if (x26c_lifeTime != 0.f) {
    float alpha = 1.f;
    if (x26c_lifeTime < 2.f)
      alpha = 1.f - (x26c_lifeTime / x270_curTime);
    else if ((x26c_lifeTime - x270_curTime) < 2.f)
      alpha = (x26c_lifeTime - x270_curTime) * 0.5f;

    drawFlags = CModelFlags(5, 0, 3, zeus::CColor(1.f, alpha));
    drawFlags.x2_flags &= 0xFFFC;
    drawFlags.x2_flags |= 1;
  }

  xb4_drawFlags = drawFlags;

  if (x64_modelData && x64_modelData->HasAnimData()) {
    SAdvancementDeltas deltas = UpdateAnimation(dt, mgr, true);
    MoveToOR(deltas.x0_posDelta, dt);
    RotateToOR(deltas.xc_rotDelta, dt);
  }

  if (x28c_25_inTractor) {
    zeus::CVector3f posDelta = mgr.GetPlayer().GetTranslation() + (2.f * zeus::skUp) - GetTranslation();
    x274_tractorTime += dt;
    posDelta = (20.f * (0.5f * zeus::min(2.f, x274_tractorTime))) * posDelta.normalized();

    if (x28c_26_enableTractorTest && (mgr.GetPlayer().GetPlayerGun()->IsCharging() ?
        mgr.GetPlayer().GetPlayerGun()->GetChargeBeamFactor() : 0.f) < CPlayerGun::skTractorBeamFactor) {
      x28c_26_enableTractorTest = false;
      x28c_25_inTractor = false;
      posDelta.zeroOut();
    }
    SetVelocityWR(posDelta);
  } else if (x28c_24_generated) {
    float chargeFactor =
        mgr.GetPlayer().GetPlayerGun()->IsCharging() ? mgr.GetPlayer().GetPlayerGun()->GetChargeBeamFactor() : 0.f;

    if (chargeFactor > CPlayerGun::skTractorBeamFactor) {
      zeus::CVector3f posDelta = GetTranslation() - mgr.GetCameraManager()->GetFirstPersonCamera()->GetTranslation();
      float relFov = zeus::CRelAngle(zeus::degToRad(g_tweakGame->GetFirstPersonFOV())).asRel();
      if (mgr.GetCameraManager()->GetFirstPersonCamera()->GetTransform().
          frontVector().dot(posDelta.normalized()) > std::cos(relFov) &&
          posDelta.magSquared() < (30.f * 30.f)) {
        x28c_25_inTractor = true;
        x28c_26_enableTractorTest = true;
        x274_tractorTime = 0.f;
      }
    }
  }

  if (x26c_lifeTime != 0.f && x270_curTime > x26c_lifeTime)
    mgr.FreeScriptObject(GetUniqueId());
}

void CScriptPickup::Touch(CActor& act, CStateManager& mgr) {
  if (GetActive() && x278_delayTimer < 0.f && TCastToPtr<CPlayer>(act)) {
    if (x258_itemType >= CPlayerState::EItemType::Truth && x258_itemType <= CPlayerState::EItemType::Newborn) {
      CAssetId id = MP1::CArtifactDoll::GetArtifactHeadScanFromItemType(x258_itemType);
      if (id.IsValid())
        mgr.GetPlayerState()->SetScanTime(id, 0.5f);
    }

    if (x27c_pickupParticleDesc) {
      if (mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::EPlayerVisor::Thermal) {
        mgr.AddObject(new CExplosion(x27c_pickupParticleDesc, mgr.AllocateUniqueId(), true,
                                     CEntityInfo(GetAreaIdAlways(), CEntity::NullConnectionList, kInvalidEditorId),
                                     "Explosion - Pickup Effect", x34_transform, 0, zeus::skOne3f,
                                     zeus::skWhite));
      }
    }

    mgr.GetPlayerState()->AddPowerUp(x258_itemType, x260_capacity);
    mgr.GetPlayerState()->IncrPickup(x258_itemType, x25c_amount);
    mgr.FreeScriptObject(GetUniqueId());
    SendScriptMsgs(EScriptObjectState::Arrived, mgr, EScriptObjectMessage::None);

    if (x260_capacity > 0) {
      u32 total = mgr.GetPlayerState()->GetPickupTotal();
      u32 colRate = mgr.GetPlayerState()->CalculateItemCollectionRate();
      if (total == colRate) {
        CPersistentOptions& opts = g_GameState->SystemOptions();
        mgr.QueueMessage(
            mgr.GetHUDMessageFrameCount() + 1,
            g_ResFactory
                ->GetResourceIdByName(opts.GetAllItemsCollected() ? "STRG_AllPickupsFound_2" : "STRG_AllPickupsFound_1")
                ->id,
            0.f);
        opts.SetAllItemsCollected(true);
      }
    }

    if (x258_itemType == CPlayerState::EItemType::PowerBombs &&
        g_GameState->SystemOptions().GetShowPowerBombAmmoMessage()) {
      g_GameState->SystemOptions().IncrementPowerBombAmmoCount();
      MP1::CSamusHud::DisplayHudMemo(g_MainStringTable->GetString(109), {0.5f, true, false, false});
    }
  }
}
} // namespace urde
