#include "Runtime/MP1/World/CGrenadeLauncher.hpp"

#include "Runtime/Character/CPASAnimParm.hpp"
#include "Runtime/Character/CPASAnimParmData.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Weapon/CGameProjectile.hpp"
#include "Runtime/World/CExplosion.hpp"
#include "Runtime/World/CPatterned.hpp"
#include "Runtime/World/CPlayer.hpp"

namespace urde::MP1 {
CGrenadeLauncher::CGrenadeLauncher(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                   const zeus::CTransform& xf, CModelData&& mData, const zeus::CAABox& bounds,
                                   const CHealthInfo& healthInfo, const CDamageVulnerability& vulnerability,
                                   const CActorParameters& actParams, TUniqueId parentId,
                                   const SGrenadeLauncherData& data, float f1)
: CPhysicsActor(uid, true, name, info, xf, std::move(mData), {EMaterialTypes::Character, EMaterialTypes::Solid}, bounds,
                SMoverData{1000.f}, actParams, 0.3f, 0.1f)
, x25c_healthInfo(healthInfo)
, x264_vulnerability(vulnerability)
, x2cc_parentId(parentId)
, x2d0_data(data)
, x328_cSphere({{}, mData.GetScale().z()}, {EMaterialTypes::Character, EMaterialTypes::Solid})
, x350_grenadeActorParams(actParams)
, x3e8_thermalMag(actParams.GetThermalMag())
, x3f8_explodePlayerDistance(f1) {
  if (data.GetExplosionGenDescId().IsValid()) {
    x3b8_particleGenDesc = g_SimplePool->GetObj({SBIG('PART'), data.GetExplosionGenDescId()});
  }
  GetModelData()->EnableLooping(true);
  const CPASDatabase& pasDatabase = GetModelData()->GetAnimationData()->GetCharacterInfo().GetPASDatabase();
  for (size_t i = 0; i < x3c8_animIds.size(); ++i) {
    const auto result = pasDatabase.FindBestAnimation(CPASAnimParmData{22, CPASAnimParm::FromEnum(int(i))}, -1);
    x3c8_animIds[i] = result.second;
  }
}

zeus::CVector3f CGrenadeLauncher::GrenadeTarget(const CStateManager& mgr) {
  const zeus::CVector3f aim = mgr.GetPlayer().GetAimPosition(mgr, 1.f);
  if (mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed) {
    return aim - zeus::CVector3f{0.f, 0.f, 0.5f * mgr.GetPlayer().GetEyeHeight()};
  }
  return aim;
}

void CGrenadeLauncher::CalculateGrenadeTrajectory(const zeus::CVector3f& target, const zeus::CVector3f& origin,
                                                  const SGrenadeTrajectoryInfo& info, float& angleOut,
                                                  float& velocityOut) {
  float angle = info.GetAngleMin();
  float velocity = info.GetVelocityMin();
  float delta = std::max(0.01f, 0.1f * (info.GetAngleMax() - info.GetAngleMin()));
  zeus::CVector3f dist = target - origin;
  float distXYMag = dist.toVec2f().magnitude();
  float velocityMinSq = info.GetVelocityMin() * info.GetVelocityMin();
  float velocityMaxSq = info.GetVelocityMax() * info.GetVelocityMax();
  float gravAdj = distXYMag * ((0.5f * CPhysicsActor::GravityConstant()) * distXYMag);
  float currAngle = info.GetAngleMin();
  float leastResult = FLT_MAX;
  while (info.GetAngleMax() >= currAngle) {
    float cos = std::cos(currAngle);
    float sin = std::sin(currAngle);
    float result = (distXYMag * (cos * sin) - (dist.z() * (cos * cos)));
    if (result > FLT_EPSILON) {
      float div = gravAdj / result;
      if (velocityMinSq <= result && result <= velocityMaxSq) {
        angle = currAngle;
        velocity = std::sqrt(div);
        break;
      }
      if (result <= velocityMaxSq) {
        result = velocityMinSq - result;
      } else {
        result = result - velocityMaxSq;
      }
      if (result < leastResult) {
        angle = currAngle;
        velocity = std::sqrt(div);
        leastResult = result;
      }
    }
    currAngle += delta;
  }
  angleOut = angle;
  velocityOut = velocity;
}

void CGrenadeLauncher::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, uid, mgr);
  switch (msg) {
  case EScriptObjectMessage::Start:
    if (x2cc_parentId == uid && x258_started != 1) {
      x258_started = 1;
      sub_80230438();
    }
    break;
  case EScriptObjectMessage::Stop:
    if (x2cc_parentId == uid && x258_started != 0) {
      x258_started = 0;
      sub_80230438();
    }
    break;
  case EScriptObjectMessage::Action:
    if (x2cc_parentId == uid && x258_started == 1) {
      x3fc_launchGrenade = true;
    }
    break;
  case EScriptObjectMessage::Registered:
    sub_80230438();
    break;
  case EScriptObjectMessage::Damage:
    x3ec_damageTimer = 0.33f;
    break;
  default:
    break;
  }
}

void CGrenadeLauncher::AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) {
  CActor::AddToRenderer(frustum, mgr);
}

std::optional<zeus::CAABox> CGrenadeLauncher::GetTouchBounds() const {
  return x328_cSphere.CalculateAABox(GetTransform());
}

void CGrenadeLauncher::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) {
  if (x3f4_color3.a() == 1.f) {
    xb4_drawFlags = CModelFlags{2, 0, 3, zeus::skWhite};
    // Original code redundantly sets a() = 1.f
    xb4_drawFlags.addColor = x3f4_color3;
  } else {
    xb4_drawFlags = CModelFlags{5, 0, 3, x3f4_color3};
  }
  CActor::PreRender(mgr, frustum);
}

void CGrenadeLauncher::Render(CStateManager& mgr) {
  if (x3fd_visible) {
    CPhysicsActor::Render(mgr);
  }
}

void CGrenadeLauncher::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    if (x3fc_launchGrenade) {
      LaunchGrenade(mgr);
      x3fc_launchGrenade = false;
    }

    UpdateCollision();
    UpdateColor(dt);
    UpdateFollowPlayer(mgr, dt);
    UpdateDamageTime(dt);

    const SAdvancementDeltas& deltas = CActor::UpdateAnimation(dt, mgr, true);
    MoveToOR(deltas.x0_posDelta, dt);
    RotateToOR(deltas.xc_rotDelta, dt);

    TCastToPtr<CPatterned> parent = mgr.ObjectById(x2cc_parentId);
    if (parent == nullptr || !parent->IsAlive() || parent->HealthInfo(mgr)->GetHP() <= 0.f) {
      mgr.SendScriptMsg(parent, GetUniqueId(), EScriptObjectMessage::Damage);
      CreateExplosion(mgr);
      mgr.FreeScriptObject(GetUniqueId());
    }
  }
}

void CGrenadeLauncher::Touch(CActor& act, CStateManager& mgr) {
  if (TCastToPtr<CGameProjectile> projectile = act) {
    if (projectile->GetOwnerId() == mgr.GetPlayer().GetUniqueId() &&
        GetDamageVulnerability()->WeaponHurts(CWeaponMode{projectile->GetType()}, false)) {
      x348_shotTimer = 0.5f;
      CEntity* parent = mgr.ObjectById(x2cc_parentId);
      if (parent != nullptr) {
        mgr.SendScriptMsg(parent, GetUniqueId(), EScriptObjectMessage::Touched);
      }
    }
  }
}

void CGrenadeLauncher::UpdateCollision() {
  x328_cSphere.SetSphereCenter(GetLocatorTransform("lockon_target_LCTR"sv).origin);
}

void CGrenadeLauncher::UpdateColor(float arg) {
  if (x348_shotTimer > 0.f) {
    x348_shotTimer = std::max(0.f, x348_shotTimer - arg);
    x34c_color1 = zeus::CColor::lerp(zeus::skWhite, zeus::skRed, x348_shotTimer);
  }
}

void CGrenadeLauncher::UpdateDamageTime(float arg) {
  if (x3ec_damageTimer <= 0.f) {
    xd0_damageMag = x3e8_thermalMag;
  } else {
    x3ec_damageTimer = std::max(0.f, x3ec_damageTimer - arg);
    x3f4_color3 = zeus::CColor::lerp(zeus::skBlack, x3f0_color2, std::clamp(x3ec_damageTimer / 0.33f, 0.f, 1.f));
    xd0_damageMag = 5.f * x3ec_damageTimer + x3e8_thermalMag;
  }
}

void CGrenadeLauncher::CreateExplosion(CStateManager& mgr) {
  if (!x3b8_particleGenDesc) {
    return;
  }
  mgr.AddObject(new CExplosion(*x3b8_particleGenDesc, mgr.AllocateUniqueId(), true,
                               {GetAreaIdAlways(), CEntity::NullConnectionList}, "Grenade Launcher Explode Fx"sv,
                               GetTransform(), 0, GetModelData()->GetScale(), zeus::skWhite));
  CSfxManager::SfxStart(x2d0_data.GetExplosionSfx(), 1.f, 1.f, false, 0x7f, false, kInvalidAreaId);
}

void CGrenadeLauncher::UpdateFollowPlayer(CStateManager& mgr, float dt) {
  CModelData* modelData = GetModelData();
  CAnimData* animData = nullptr;

  if (modelData != nullptr && (animData = modelData->GetAnimationData()) != nullptr && x258_started == 1 &&
      x3fe_followPlayer) {
    const zeus::CVector3f target = mgr.GetPlayer().GetAimPosition(mgr, 0.f) - GetTranslation();
    const zeus::CVector3f rot = GetTransform().transposeRotate({target.x(), target.y(), 0.f});

    if (rot.canBeNormalized()) {
      constexpr float p36d = zeus::degToRad(36.476f);
      constexpr float n45d = zeus::degToRad(-45.f);
      constexpr float p45d = zeus::degToRad(45.f);

      float l84 = p36d * std::clamp(std::atan2(rot.x(), rot.y()), n45d, p45d);
      float l88 = std::clamp((0.25f * (l84 - x3d8_)) / dt, -3.f, 3.f);
      float l8c = std::clamp((l88 - x3dc_) / dt, -10.f, 10.f);
      x3dc_ += dt * l8c;

      float l90 = p36d * std::clamp(std::atan2(rot.z(), rot.toVec2f().magnitude()), n45d, p45d);
      l88 = std::clamp((0.25f * (l90 - x3e0_)) / dt, -3.f, 3.f);
      l8c = std::clamp((l88 - x3e4_) / dt, -10.f, 10.f);
      x3e4_ += dt * l8c;

      float dVar7 = std::clamp(dt * x3dc_ + x3d8_, -0.5f, 0.5f);
      float dVar8 = std::clamp(dt * x3e4_ + x3e0_, -0.5f, 0.5f);

      if (dVar7 != x3d8_) {
        if (std::abs(x3d8_) > 0.f && x3d8_ * dVar7 <= 0.f) {
          animData->DelAdditiveAnimation(x3c8_animIds[x3d8_ >= 0.f ? 1 : 0]);
        }
        float weight = std::abs(dVar7);
        if (weight > 0.f) {
          animData->AddAdditiveAnimation(x3c8_animIds[dVar7 >= 0.f ? 1 : 0], weight, false, false);
        }
      }
      if (dVar8 != x3e0_) {
        if (std::abs(x3e0_) > 0.f && x3e0_ * dVar8 <= 0.f) {
          animData->DelAdditiveAnimation(x3c8_animIds[x3e0_ <= 0.f ? 3 : 2]);
        }
        float weight = std::abs(dVar8);
        if (weight > 0.f) {
          animData->AddAdditiveAnimation(x3c8_animIds[dVar8 <= 0.f ? 3 : 2], weight, false, false);
        }
      }
      x3d8_ = dVar7;
      x3e0_ = dVar8;
    }
  } else {
    if (x3d8_ != 0.f) {
      if (animData != nullptr) { // Original code does not check
        animData->DelAdditiveAnimation(x3c8_animIds[x3d8_ >= 0.f ? 1 : 0]);
      }
      x3d8_ = 0.f;
    }
    if (x3e0_ != 0.f) {
      if (animData != nullptr) { // Original code does not check
        animData->DelAdditiveAnimation(x3c8_animIds[x3e0_ <= 0.f ? 3 : 2]);
      }
      x3e0_ = 0.f;
    }
  }
}

void CGrenadeLauncher::sub_80230438() {
  CModelData* modelData = GetModelData();
  CAnimData* animData;
  if (modelData == nullptr || (animData = modelData->GetAnimationData()) == nullptr || x258_started <= -1 ||
      x258_started >= 2) {
    return;
  }

  constexpr std::array arr{0, 3};
  const auto anim = animData->GetCharacterInfo().GetPASDatabase().FindBestAnimation(
      CPASAnimParmData{5, CPASAnimParm::FromEnum(0), CPASAnimParm::FromEnum(arr[x258_started])}, -1);
  if (anim.first > 0.f) {
    animData->SetAnimation({anim.second, -1, 1.f, true}, false);
    modelData->EnableLooping(true);
  }
}

void CGrenadeLauncher::LaunchGrenade(CStateManager& mgr) {
  CModelData* modelData = GetModelData();
  CAnimData* animData;
  if (modelData == nullptr || (animData = modelData->GetAnimationData()) == nullptr) {
    return;
  }

  const auto& anim = animData->GetCharacterInfo().GetPASDatabase().FindBestAnimation(CPASAnimParmData{23}, -1);
  if (anim.first > 0.f) {
    animData->AddAdditiveAnimation(anim.second, 1.f, false, true);
    const zeus::CVector3f origin =
        GetTranslation() + GetTransform().rotate(GetLocatorTransform("grenade_LCTR"sv).origin);
    const zeus::CVector3f target = GrenadeTarget(mgr);
    float angleOut = x2d0_data.GetGrenadeTrajectoryInfo().GetAngleMin();
    float velocityOut = x2d0_data.GetGrenadeTrajectoryInfo().GetVelocityMin();
    CalculateGrenadeTrajectory(target, origin, x2d0_data.GetGrenadeTrajectoryInfo(), angleOut, velocityOut);

    zeus::CVector3f dist = target - origin;
    dist.z() = 0.f;
    const zeus::CVector3f front = GetTransform().frontVector();
    if (dist.canBeNormalized()) {
      dist.normalize();
    } else {
      dist = front;
    }

    constexpr float maxAngle = zeus::degToRad(45.f);
    if (zeus::CVector3f::getAngleDiff(front, dist) > maxAngle) {
      dist = zeus::CVector3f::slerp(front, dist, maxAngle);
    }

    const zeus::CVector3f look = zeus::CVector3f::slerp(dist, zeus::skUp, angleOut);
    const zeus::CTransform xf = zeus::lookAt(origin, origin + look, zeus::skUp);
    CModelData mData{CStaticRes{x2d0_data.GetGrenadeModelId(), GetModelData()->GetScale()}};
    mgr.AddObject(new CBouncyGrenade(mgr.AllocateUniqueId(), "Bouncy Grenade"sv,
                                     {GetAreaIdAlways(), CEntity::NullConnectionList}, xf, std::move(mData),
                                     x350_grenadeActorParams, x2cc_parentId, x2d0_data.GetGrenadeData(), velocityOut,
                                     x3f8_explodePlayerDistance));
  }
}
} // namespace urde::MP1
