#include "Runtime/Weapon/CBeamProjectile.hpp"

#include "Runtime/CStateManager.hpp"
#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

CBeamProjectile::CBeamProjectile(const TToken<CWeaponDescription>& wDesc, std::string_view name, EWeaponType wType,
                                 const zeus::CTransform& xf, s32 maxLength, float beamRadius, float travelSpeed,
                                 EMaterialTypes matType, const CDamageInfo& dInfo, TUniqueId uid, TAreaId aid,
                                 TUniqueId owner, EProjectileAttrib attribs, bool growingBeam)
: CGameProjectile(false, wDesc, name, wType, xf, matType, dInfo, uid, aid, owner, kInvalidUniqueId, attribs, false,
                  zeus::skOne3f, {}, 0xffff, false)
, x2e8_intMaxLength(std::abs(maxLength))
, x2ec_maxLength(x2e8_intMaxLength)
, x2f0_invMaxLength(1.f / x2ec_maxLength)
, x2f4_beamRadius(beamRadius)
, x300_intBeamLength(growingBeam ? 0.f : x2ec_maxLength)
, x304_beamLength(x2ec_maxLength)
, x308_travelSpeed(travelSpeed)
, x464_24_growingBeam(growingBeam) {
  x384_.resize(10);
  x400_pointCache.resize(8);
}

std::optional<zeus::CAABox> CBeamProjectile::GetTouchBounds() const {
  if (!GetActive()) {
    return std::nullopt;
  }

  if (x464_25_enableTouchDamage) {
    const zeus::CVector3f pos = GetTranslation();
    return {{pos - 0.1f, pos + 0.1f}};
  }

  return std::nullopt;
}

void CBeamProjectile::CalculateRenderBounds() { x9c_renderBounds = x354_.getTransformedAABox(x324_xf); }

void CBeamProjectile::ResetBeam(CStateManager&, bool) {
  if (x464_24_growingBeam)
    x300_intBeamLength = 0.f;
}

void CBeamProjectile::SetCollisionResultData(EDamageType dType, CRayCastResult& res, TUniqueId id) {
  x2f8_damageType = dType;
  x304_beamLength = res.GetT();
  x318_collisionPoint = res.GetPoint();
  x30c_collisionNormal = res.GetPlane().normal();
  x2fe_collisionActorId = dType == EDamageType::Actor ? id : kInvalidUniqueId;
  SetTranslation(res.GetPoint());
}

void CBeamProjectile::UpdateFx(const zeus::CTransform& xf, float dt, CStateManager& mgr) {
  if (!GetActive())
    return;

  SetTransform(xf.getRotation());
  if (x464_24_growingBeam) {
    x300_intBeamLength += x308_travelSpeed * dt;
    if (x300_intBeamLength > x2ec_maxLength)
      x300_intBeamLength = x2ec_maxLength;
  }
  x304_beamLength = x300_intBeamLength;
  x2f8_damageType = EDamageType::None;
  x298_previousPos = xf.origin;
  zeus::CVector3f beamEnd = xf.basis[1].normalized() * x300_intBeamLength + xf.origin;
  SetTranslation(beamEnd);
  x354_ = zeus::CAABox(zeus::CVector3f{-x2f4_beamRadius, 0.f, -x2f4_beamRadius},
                       zeus::CVector3f{x2f4_beamRadius, x304_beamLength, x2f4_beamRadius});
  x36c_ = zeus::CAABox(zeus::CVector3f{-x2f4_beamRadius, 0.f, -x2f4_beamRadius},
                       zeus::CVector3f{x2f4_beamRadius, x300_intBeamLength, x2f4_beamRadius}).getTransformedAABox(xf);
  rstl::reserved_vector<TUniqueId, 1024> nearList;
  mgr.BuildNearList(nearList, x36c_, CMaterialFilter::MakeExclude({EMaterialTypes::ProjectilePassthrough}), nullptr);
  TUniqueId collideId = kInvalidUniqueId;
  CRayCastResult res = RayCollisionCheckWithWorld(collideId, x298_previousPos, beamEnd, x300_intBeamLength, nearList, mgr);
  if (TCastToPtr<CActor> act = mgr.ObjectById(collideId)) {
    SetCollisionResultData(EDamageType::Actor, res, collideId);
    if (x464_25_enableTouchDamage)
      ApplyDamageToActors(mgr, CDamageInfo(x12c_curDamageInfo, dt));
  } else if (res.IsValid()) {
    SetCollisionResultData(EDamageType::World, res, kInvalidUniqueId);
    if (x464_25_enableTouchDamage)
      mgr.ApplyDamageToWorld(xec_ownerId, *this, res.GetPoint(), CDamageInfo(x12c_curDamageInfo, dt), xf8_filter);
  } else {
    x318_collisionPoint = xf * zeus::CVector3f(x2f4_beamRadius, x304_beamLength, x2f4_beamRadius);
    SetTranslation(x318_collisionPoint);
  }
  x324_xf = xf;
}

void CBeamProjectile::Accept(urde::IVisitor& visitor) { visitor.Visit(this); }

} // namespace urde
