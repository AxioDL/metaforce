#include "Runtime/MP1/World/CAtomicAlpha.hpp"

#include <array>

#include "Runtime/CStateManager.hpp"
#include "Runtime/Weapon/CPlayerGun.hpp"
#include "Runtime/World/CGameArea.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CWorld.hpp"

namespace urde::MP1 {
constexpr std::array skBombLocators{
    "bomb1_LCTR"sv,
    "bomb2_LCTR"sv,
    "bomb3_LCTR"sv,
    "bomb4_LCTR"sv,
};

CAtomicAlpha::CAtomicAlpha(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                           CModelData&& mData, const CActorParameters& actParms, const CPatternedInfo& pInfo,
                           CAssetId bombWeapon, const CDamageInfo& bombDamage, float bombDropDelay, float f2, float f3,
                           CAssetId cmdl, bool invisible, bool b2)
: CPatterned(ECharacter::AtomicAlpha, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Flyer, EColliderType::One, EBodyType::Flyer, actParms, EKnockBackVariant::Medium)
, x568_25_invisible(invisible)
, x568_26_applyBeamAttraction(b2)
, x56c_bomdDropDelay(bombDropDelay)
, x570_bombReappearDelay(f2)
, x574_bombRappearTime(f3)
, x580_pathFind(nullptr, 3, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, x668_bombProjectile(bombWeapon, bombDamage)
, x690_bombModel(CStaticRes(cmdl, GetModelData()->GetScale())) {
  x668_bombProjectile.Token().Lock();
  for (u32 i = 0; i < skBombCount; ++i) {
    x6dc_bombLocators.emplace_back(skBombLocators[i], pas::ELocomotionType(u32(pas::ELocomotionType::Internal10) + i));
  }
}

void CAtomicAlpha::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
  if (msg == EScriptObjectMessage::InitializedInArea) {
    x580_pathFind.SetArea(mgr.GetWorld()->GetAreaAlways(GetAreaIdAlways())->GetPostConstructed()->x10bc_pathArea);
  } else if (msg == EScriptObjectMessage::Registered) {
    x450_bodyController->Activate(mgr);
  } else if (msg == EScriptObjectMessage::AddSplashInhabitant) {
    if (x400_25_alive)
      x401_30_pendingDeath = true;
  }
}

void CAtomicAlpha::Render(CStateManager& mgr) {
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::EPlayerVisor::XRay && x568_25_invisible)
    return;

  CPatterned::Render(mgr);
  for (const SBomb& bomb : x6dc_bombLocators) {
    zeus::CTransform locatorXf =
        GetTransform() * GetScaledLocatorTransform(bomb.x0_locatorName) *
        zeus::CTransform::Scale(
            std::min(1.f, std::max(0.f, bomb.x14_scaleTime - x570_bombReappearDelay) / x570_bombReappearDelay));
    CModelFlags flags;
    flags.x2_flags = 1 | 2;
    flags.x4_color = zeus::skWhite;
    x690_bombModel.Render(mgr, locatorXf, x90_actorLights.get(), flags);
  }
}
void CAtomicAlpha::AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) {
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::EPlayerVisor::XRay && x568_25_invisible) {
    return;
  }
  CPatterned::AddToRenderer(frustum, mgr);
}

void CAtomicAlpha::Think(float dt, CStateManager& mgr) {
  CPatterned::Think(dt, mgr);
  if (!GetActive())
    return;

  x578_bombTime += dt;

  for (SBomb& bomb : x6dc_bombLocators) {
    bomb.x14_scaleTime += dt;
  }
}

void CAtomicAlpha::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  if (type == EUserEventType::Projectile) {
    zeus::CVector3f origin = GetLctrTransform(node.GetLocatorName()).origin;
    zeus::CTransform xf = zeus::lookAt(origin, origin + zeus::skDown, zeus::skUp);
    LaunchProjectile(xf, mgr, 4, EProjectileAttrib::None, false, {}, 0xFFFF, false, zeus::CVector3f(1.f));
    x578_bombTime = 0.f;
    x57c_curBomb = (x57c_curBomb + 1) & (x6dc_bombLocators.size() - 1);
  } else
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}

bool CAtomicAlpha::Leash(CStateManager& mgr, float) {
  if ((mgr.GetPlayer().GetTranslation() - GetTranslation()).magSquared() <=
      x3cc_playerLeashRadius * x3cc_playerLeashRadius)
    return false;

  return x3d4_curPlayerLeashTime > x3d0_playerLeashTime;
}

bool CAtomicAlpha::AggressionCheck(CStateManager& mgr, float) {
  const CPlayerGun* playerGun = mgr.GetPlayer().GetPlayerGun();
  float factor = 0.f;
  if (x568_26_applyBeamAttraction && playerGun->IsCharging())
    factor = playerGun->GetChargeBeamFactor();
  return factor > 0.1f;
}

void CAtomicAlpha::CollidedWith(TUniqueId uid, const CCollisionInfoList& list, CStateManager& mgr) {
  if (IsAlive()) {
    if (TCastToConstPtr<CPlayer> pl = mgr.GetObjectById(uid)) {
      if (x420_curDamageRemTime <= 0.f) {
        mgr.GetPlayerState()->GetStaticInterference().AddSource(GetUniqueId(), 0.5f, 0.25f);
        for (SBomb& bomb : x6dc_bombLocators) {
          bomb.x14_scaleTime = 0.f;
        }
      }
    }
  }
  CPatterned::CollidedWith(uid, list, mgr);
}

void CAtomicAlpha::Patrol(CStateManager& mgr, EStateMsg msg, float arg) {
  CPatterned::Patrol(mgr, msg, arg);
  if (msg == EStateMsg::Activate) {
    x578_bombTime = 0.f;
  } else if (msg == EStateMsg::Update) {
    if (x568_24_inRange) {
      if (x578_bombTime >= x56c_bomdDropDelay &&
          x6dc_bombLocators[0].x14_scaleTime > (x570_bombReappearDelay + x574_bombRappearTime)) {
        x450_bodyController->SetLocomotionType(x6dc_bombLocators[x57c_curBomb].x10_locomotionType);
      } else {
        x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
      }
      if (Leash(mgr, arg))
        x568_24_inRange = false;
    } else {
      x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
      if (InMaxRange(mgr, arg))
        x568_24_inRange = true;
    }
  } else if (msg == EStateMsg::Deactivate) {
    x568_24_inRange = false;
  }
}

void CAtomicAlpha::Attack(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Internal8);
  } else if (msg == EStateMsg::Update) {
    zeus::CVector3f seekVec = x664_steeringBehaviors.Seek(*this, mgr.GetPlayer().GetEyePosition());
    x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(seekVec, {}, 1.f));
  } else if (msg == EStateMsg::Deactivate) {
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
  }
}
} // namespace urde::MP1
