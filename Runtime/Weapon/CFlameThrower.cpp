#include "Runtime/Weapon/CFlameThrower.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Collision/CInternalRayCastStructure.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/Weapon/CFlameInfo.hpp"
#include "Runtime/Weapon/CFlameThrower.hpp"
#include "Runtime/World/CGameLight.hpp"
#include "Runtime/World/CPlayer.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {
const zeus::CVector3f CFlameThrower::kLightOffset(0, 3.f, 2.f);

CFlameThrower::CFlameThrower(const TToken<CWeaponDescription>& wDesc, std::string_view name, EWeaponType wType,
                             const CFlameInfo& flameInfo, const zeus::CTransform& xf, EMaterialTypes matType,
                             const CDamageInfo& dInfo, TUniqueId uid, TAreaId aId, TUniqueId owner,
                             EProjectileAttrib attribs, CAssetId playerSteamTxtr, s16 playerHitSfx,
                             CAssetId playerIceTxtr)
: CGameProjectile(false, wDesc, name, wType, xf, matType, dInfo, uid, aId, owner, kInvalidUniqueId, attribs, false,
                  zeus::CVector3f(1.f), {}, -1, false)
, x2e8_flameXf(xf)
, x338_(flameInfo.x10_)
, x33c_flameDesc(g_SimplePool->GetObj({FOURCC('PART'), flameInfo.GetFlameFxId()}))
, x348_flameGen(std::make_unique<CElementGen>(x33c_flameDesc))
, x34c_flameWarp(176.f - float(flameInfo.GetLength()), xf.origin, bool(flameInfo.GetAttributes() & 0x4))
, x3f4_playerSteamTxtr(playerSteamTxtr)
, x3f8_playerHitSfx(playerHitSfx)
, x3fc_playerIceTxtr(playerIceTxtr)
, x400_26_((flameInfo.GetAttributes() & 1) == 0)
, x400_27_coneCollision((flameInfo.GetAttributes() & 0x2) != 0) {}

void CFlameThrower::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CFlameThrower::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  if (msg == EScriptObjectMessage::Registered) {
    xe6_27_thermalVisorFlags |= 2;
    mgr.AddWeaponId(xec_ownerId, xf0_weaponType);
  } else if (msg == EScriptObjectMessage::Deleted) {
    mgr.RemoveWeaponId(xec_ownerId, xf0_weaponType);
    DeleteProjectileLight(mgr);
  }

  CGameProjectile::AcceptScriptMsg(msg, uid, mgr);
}

void CFlameThrower::SetTransform(const zeus::CTransform& xf, float) { x2e8_flameXf = xf; }

void CFlameThrower::Reset(CStateManager& mgr, bool resetWarp) {
  SetFlameLightActive(mgr, false);
  if (resetWarp) {
    SetActive(false);
    x400_25_particlesActive = false;
    x3f0_flameState = EFlameState::Default;
    x330_particleWaitDelayTimer = 0.f;
    x334_fireStopTimer = 0.f;
    x318_flameBounds = zeus::skNullBox;
    x348_flameGen->SetParticleEmission(false);
    x34c_flameWarp.ResetPosition(x2e8_flameXf.origin);
  } else {
    x348_flameGen->SetParticleEmission(false);
    x400_25_particlesActive = false;
    x3f0_flameState = EFlameState::FireStopTimer;
  }
}

void CFlameThrower::Fire(const zeus::CTransform&, CStateManager& mgr, bool) {
  SetActive(true);
  x400_25_particlesActive = true;
  x400_24_active = true;
  x3f0_flameState = EFlameState::FireStart;
  CreateFlameParticles(mgr);
}

void CFlameThrower::CreateFlameParticles(CStateManager& mgr) {
  DeleteProjectileLight(mgr);
  x348_flameGen = std::make_unique<CElementGen>(x33c_flameDesc);
  x348_flameGen->SetParticleEmission(true);
  x348_flameGen->SetZTest(x400_27_coneCollision);
  x348_flameGen->AddModifier(&x34c_flameWarp);
  if (x348_flameGen->SystemHasLight() && x2c8_projectileLight == kInvalidUniqueId)
    CreateProjectileLight("FlameThrower_Light"sv, x348_flameGen->GetLight(), mgr);
}

void CFlameThrower::AddToRenderer(const zeus::CFrustum&, CStateManager& mgr) {
  g_Renderer->AddParticleGen(*x348_flameGen);
  EnsureRendered(mgr, x2e8_flameXf.origin, GetRenderBounds());
}

void CFlameThrower::Render(CStateManager&) {}

std::optional<zeus::CAABox> CFlameThrower::GetTouchBounds() const { return std::nullopt; }

void CFlameThrower::Touch(CActor&, CStateManager&) {}

void CFlameThrower::SetFlameLightActive(CStateManager& mgr, bool active) {
  if (x2c8_projectileLight == kInvalidUniqueId)
    return;

  if (TCastToPtr<CGameLight> light = mgr.ObjectById(x2c8_projectileLight))
    light->SetActive(active);
}

void CFlameThrower::UpdateFlameState(float dt, CStateManager& mgr) {
  switch(x3f0_flameState) {
  case EFlameState::FireStart:
    x3f0_flameState = EFlameState::FireActive;
    break;
  case EFlameState::FireStopTimer:
    x334_fireStopTimer += 4.f * dt;
    if (x334_fireStopTimer > 1.f) {
      x334_fireStopTimer = 1.f;
      x3f0_flameState = EFlameState::FireWaitForParticlesDone;
      x400_24_active = false;
    }
    break;
  case EFlameState::FireWaitForParticlesDone:
    x330_particleWaitDelayTimer += dt;
    if (x330_particleWaitDelayTimer > 0.1f && x348_flameGen && x348_flameGen->GetParticleCountAll() == 0) {
      x3f0_flameState = EFlameState::Default;
      Reset(mgr, true);
    }
    break;
  default:
    break;
  }
}

CRayCastResult CFlameThrower::DoCollisionCheck(TUniqueId& idOut, const zeus::CAABox& aabb, CStateManager& mgr) {
  CRayCastResult ret;
  rstl::reserved_vector<TUniqueId, 1024> nearList;
  mgr.BuildNearList(nearList, aabb, CMaterialFilter::skPassEverything, this);
  const auto& colPoints = x34c_flameWarp.GetCollisionPoints();

  if (x400_27_coneCollision && !colPoints.empty()) {
    const float radiusPitch =
        (x34c_flameWarp.GetMaxSize() - x34c_flameWarp.GetMinSize()) / float(colPoints.size()) * 0.5f;
    float curRadius = radiusPitch;

    for (size_t i = 1; i < colPoints.size(); ++i) {
      const zeus::CVector3f delta = colPoints[i] - colPoints[i - 1];
      zeus::CTransform lookXf = zeus::lookAt(colPoints[i - 1], colPoints[i]);
      lookXf.origin = delta * 0.5f + colPoints[i - 1];
      const zeus::COBBox obb(lookXf, {curRadius, delta.magnitude() * 0.5f, curRadius});

      for (const TUniqueId id : nearList) {
        if (auto* act = static_cast<CActor*>(mgr.ObjectById(id))) {
          const CProjectileTouchResult tres = CanCollideWith(*act, mgr);
          if (tres.GetActorId() == kInvalidUniqueId) {
            continue;
          }

          const auto tb = act->GetTouchBounds();
          if (!tb) {
            continue;
          }

          if (obb.AABoxIntersectsBox(*tb)) {
            const CCollidableAABox caabb(*tb, act->GetMaterialList());
            const zeus::CVector3f flameToAct = act->GetAimPosition(mgr, 0.f) - x2e8_flameXf.origin;
            const float flameToActDist = flameToAct.magnitude();
            const CInternalRayCastStructure rc(x2e8_flameXf.origin, flameToAct.normalized(), flameToActDist, {},
                                               CMaterialFilter::skPassEverything);
            const CRayCastResult cres = caabb.CastRayInternal(rc);
            if (cres.IsInvalid()) {
              continue;
            }
            return cres;
          }
        }
      }

      curRadius += radiusPitch;
    }
  } else {
    for (size_t i = 0; i < colPoints.size() - 1; ++i) {
      const zeus::CVector3f delta = colPoints[i + 1] - colPoints[i];
      const float deltaMag = delta.magnitude();
      if (deltaMag <= 0.f) {
        break;
      }

      const CRayCastResult cres = RayCollisionCheckWithWorld(idOut, colPoints[i], colPoints[i + 1], deltaMag, nearList, mgr);
      if (cres.IsValid()) {
        return cres;
      }
    }
  }

  return ret;
}

void CFlameThrower::ApplyDamageToActor(CStateManager& mgr, TUniqueId id, float dt) {
  if (mgr.GetPlayer().GetUniqueId() == id && x3f4_playerSteamTxtr.IsValid() && x3fc_playerIceTxtr.IsValid())
    mgr.GetPlayer().Freeze(mgr, x3f4_playerSteamTxtr, x3f8_playerHitSfx, x3fc_playerIceTxtr);
  CDamageInfo useDInfo = CDamageInfo(x12c_curDamageInfo, dt);
  ApplyDamageToActors(mgr, useDInfo);
}

void CFlameThrower::Think(float dt, CStateManager& mgr) {
  CWeapon::Think(dt, mgr);
  if (!GetActive())
    return;

  UpdateFlameState(dt, mgr);

  zeus::CVector3f flamePoint = x2e8_flameXf.origin;
  bool r28 = x3f0_flameState == EFlameState::FireActive || x3f0_flameState == EFlameState::FireStopTimer;
  if (r28) {
    x34c_flameWarp.Activate(true);
    x34c_flameWarp.SetWarpPoint(flamePoint);
    x34c_flameWarp.SetStateManager(mgr);
    x348_flameGen->SetTranslation(flamePoint);
    x348_flameGen->SetOrientation(x2e8_flameXf.getRotation());
  } else {
    x34c_flameWarp.Activate(false);
  }

  x348_flameGen->Update(dt);
  x34c_flameWarp.SetMaxDistSq(0.f);
  x34c_flameWarp.SetFloatingPoint(flamePoint);

  if (r28 && x34c_flameWarp.IsProcessed()) {
    x318_flameBounds = x34c_flameWarp.CalculateBounds();
    TUniqueId id = kInvalidUniqueId;
    CRayCastResult res = DoCollisionCheck(id, x318_flameBounds, mgr);
    if (TCastToPtr<CActor> act = mgr.ObjectById(id)) {
      ApplyDamageToActor(mgr, id, dt);
    } else if (res.IsValid()) {
      CMaterialFilter useFilter = xf8_filter;
      CDamageInfo useDInfo = CDamageInfo(x12c_curDamageInfo, dt);
      mgr.ApplyDamageToWorld(xec_ownerId, *this, res.GetPoint(), useDInfo, useFilter);
    }
  }

  CActor::SetTransform(x2e8_flameXf.getRotation());
  CActor::SetTranslation(x2e8_flameXf.origin);

  if (x2c8_projectileLight != kInvalidUniqueId) {
    if (TCastToPtr<CGameLight> light = mgr.ObjectById(x2c8_projectileLight)) {
      light->SetTransform(GetTransform());
      light->SetTranslation(x34c_flameWarp.GetFloatingPoint());
      if (x348_flameGen && x348_flameGen->SystemHasLight())
        light->SetLight(x348_flameGen->GetLight());
    }
  }
}
} // namespace urde
