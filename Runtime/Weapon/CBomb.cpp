#include "Runtime/Weapon/CBomb.hpp"

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/World/CGameLight.hpp"
#include "Runtime/World/CMorphBall.hpp"
#include "Runtime/World/CPlayer.hpp"

#include "DataSpec/DNAMP1/SFX/Weapons.h"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

CBomb::CBomb(const TCachedToken<CGenDescription>& particle1, const TCachedToken<CGenDescription>& particle2,
             TUniqueId uid, TAreaId aid, TUniqueId playerId, float f1, const zeus::CTransform& xf,
             const CDamageInfo& dInfo)
: CWeapon(uid, aid, true, playerId, EWeaponType::Bomb, "Bomb", xf,
          CMaterialFilter::MakeIncludeExclude(
              {EMaterialTypes::Solid, EMaterialTypes::Trigger, EMaterialTypes::NonSolidDamageable},
              {EMaterialTypes::Projectile, EMaterialTypes::Bomb}),
          {EMaterialTypes::Projectile, EMaterialTypes::Bomb}, dInfo, EProjectileAttrib::Bombs,
          CModelData::CModelDataNull())
, x17c_fuseTime(f1)
, x180_particle1(std::make_unique<CElementGen>(particle1, CElementGen::EModelOrientationType::Normal,
                                               CElementGen::EOptionalSystemFlags::One))
, x184_particle2(std::make_unique<CElementGen>(particle2, CElementGen::EModelOrientationType::Normal,
                                               CElementGen::EOptionalSystemFlags::One))
, x18c_particle2Obj(particle2.GetObj()) {
  x180_particle1->SetGlobalTranslation(xf.origin);
  x184_particle2->SetGlobalTranslation(xf.origin);
}

void CBomb::Accept(urde::IVisitor& visitor) { visitor.Visit(this); }

void CBomb::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  if (msg == EScriptObjectMessage::Registered) {

    x188_lightId = mgr.AllocateUniqueId();
    CGameLight* gameLight = new CGameLight(x188_lightId, GetAreaIdAlways(), false,
                                           std::string("Bomb_PLight") + GetName().data(), GetTransform(), GetUniqueId(),
                                           x184_particle2->GetLight(), reinterpret_cast<size_t>(x18c_particle2Obj), 1, 0.f);
    mgr.AddObject(gameLight);
    mgr.AddWeaponId(xec_ownerId, xf0_weaponType);
    CSfxManager::AddEmitter(SFXwpn_bomb_drop, GetTranslation(), {}, true, false, 0x7f, -1);
    mgr.InformListeners(GetTranslation(), EListenNoiseType::BombExplode);
    return;
  } else if (msg == EScriptObjectMessage::Deleted) {
    if (x188_lightId != kInvalidUniqueId)
      mgr.FreeScriptObject(x188_lightId);

    if (x190_24_isNotDetonated)
      mgr.RemoveWeaponId(xec_ownerId, xf0_weaponType);

    return;
  }

  CActor::AcceptScriptMsg(msg, uid, mgr);
}

constexpr CMaterialFilter kSolidFilter =
    CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {EMaterialTypes::Character, EMaterialTypes::Player,
                                                                  EMaterialTypes::ProjectilePassthrough});
void CBomb::Think(float dt, urde::CStateManager& mgr) {
  CWeapon::Think(dt, mgr);

  if (x190_24_isNotDetonated) {
    if (x17c_fuseTime <= 0.f) {
      Explode(GetTranslation(), mgr);
      if (TCastToPtr<CGameLight> light = mgr.ObjectById(x188_lightId))
        light->SetActive(true);
    }

    if (x17c_fuseTime > 0.5f)
      x180_particle1->Update(dt);
    else
      UpdateLight(dt, mgr);

    if (!x190_26_disableFuse)
      x17c_fuseTime -= dt;
  } else {
    UpdateLight(dt, mgr);
    if (x184_particle2->IsSystemDeletable())
      mgr.FreeScriptObject(GetUniqueId());
  }

  if (x190_24_isNotDetonated) {

    if (x164_acceleration.magSquared() > 0.f)
      x158_velocity += dt * x164_acceleration;

    if (x158_velocity.magSquared() > 0.f) {
      x170_prevLocation = GetTranslation();
      CActor::SetTranslation((dt * x158_velocity) + GetTranslation());

      zeus::CVector3f diffVec = (GetTranslation() - x170_prevLocation);
      float diffMag = diffVec.magnitude();
      if (diffMag == 0.f)
        Explode(GetTranslation(), mgr);
      else {
        CRayCastResult res =
            mgr.RayStaticIntersection(x170_prevLocation, (1.f / diffMag) * diffVec, diffMag, kSolidFilter);
        if (res.IsValid())
          Explode(GetTranslation(), mgr);
      }
    }
  }

  x180_particle1->SetGlobalTranslation(GetTranslation());
  x184_particle2->SetGlobalTranslation(GetTranslation());
}

void CBomb::AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) {
  zeus::CVector3f origin = GetTranslation();
  float ballRadius = mgr.GetPlayer().GetMorphBall()->GetBallRadius();

  zeus::CAABox aabox(origin - (0.9f * ballRadius), origin + (0.9f * ballRadius));
  zeus::CVector3f closestPoint = aabox.closestPointAlongVector(CGraphics::g_ViewMatrix.frontVector());

  if (x190_24_isNotDetonated && x17c_fuseTime > 0.5f)
    g_Renderer->AddParticleGen(*x180_particle1, closestPoint, aabox);
  else
    g_Renderer->AddParticleGen(*x184_particle2, closestPoint, aabox);
}

void CBomb::Touch(CActor&, urde::CStateManager&) {
#if 0
    x190_24_isNotDetonated; /* wat? */
#endif
}

std::optional<zeus::CAABox> CBomb::GetTouchBounds() const {
  float radius = (x190_24_isNotDetonated ? 0.2f : x12c_curDamageInfo.GetRadius());
  float minX = (x170_prevLocation.x() >= GetTranslation().x() ? x170_prevLocation.x() : GetTranslation().x()) - radius;
  float minY = (x170_prevLocation.y() >= GetTranslation().y() ? x170_prevLocation.y() : GetTranslation().y()) - radius;
  float minZ = (x170_prevLocation.z() >= GetTranslation().z() ? x170_prevLocation.z() : GetTranslation().z()) - radius;
  float maxX = (x170_prevLocation.x() >= GetTranslation().x() ? x170_prevLocation.x() : GetTranslation().x()) + radius;
  float maxY = (x170_prevLocation.y() >= GetTranslation().y() ? x170_prevLocation.y() : GetTranslation().y()) + radius;
  float maxZ = (x170_prevLocation.z() >= GetTranslation().z() ? x170_prevLocation.z() : GetTranslation().z()) + radius;

  return {{minX, minY, minZ, maxX, maxY, maxZ}};
}

void CBomb::Explode(const zeus::CVector3f& pos, CStateManager& mgr) {
  mgr.ApplyDamageToWorld(xec_ownerId, *this, pos, x12c_curDamageInfo, xf8_filter);
  CSfxManager::AddEmitter(SFXwpn_bomb_explo, GetTranslation(), {}, true, false, 0x7f, -1);
  mgr.InformListeners(pos, EListenNoiseType::BombExplode);
  mgr.RemoveWeaponId(xec_ownerId, xf0_weaponType);
  x190_24_isNotDetonated = false;
}
void CBomb::UpdateLight(float dt, CStateManager& mgr) {
  x184_particle2->Update(dt);
  if (x188_lightId == kInvalidUniqueId)
    return;

  if (TCastToPtr<CGameLight> light = mgr.ObjectById(x188_lightId))
    if (light->GetActive())
      light->SetLight(x184_particle2->GetLight());
}

} // namespace urde
