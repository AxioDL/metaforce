#include "MetroidPrime/Weapons/CBeamProjectile.hpp"

#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"

CBeamProjectile::CBeamProjectile(const TToken< CWeaponDescription >& wDesc,
                                 const rstl::string& name, EWeaponType wType,
                                 const CTransform4f& xf, int maxLength, float beamRadius,
                                 float travelSpeed, EMaterialTypes matType,
                                 const CDamageInfo& dInfo, TUniqueId uid, TAreaId aid,
                                 TUniqueId owner, EProjectileAttrib attribs, bool growingBeam)
: CGameProjectile(false, wDesc, name, wType, xf, matType, dInfo, uid, aid, owner, kInvalidUniqueId,
                  attribs, false, CVector3f(1.f, 1.f, 1.f), rstl::optional_object_null(),
                  CSfxManager::kInternalInvalidSfxId, false)
, x2e8_intMaxLength(CMath::AbsI(maxLength))
, x2ec_maxLength(x2e8_intMaxLength)
, x2f0_invMaxLength(1.f / x2ec_maxLength)
, x2f4_beamRadius(beamRadius)
, x2f8_damageType(kDT_None)
, x2fc_(kInvalidUniqueId)
, x2fe_collisionActorId(kInvalidUniqueId)
, x300_intBeamLength(growingBeam ? 0.f : x2ec_maxLength)
, x304_beamLength(x2ec_maxLength)
, x308_travelSpeed(travelSpeed)
, x30c_collisionNormal(CVector3f::Up())
, x318_collisionPoint(CVector3f::Zero())
, x324_xf(CTransform4f::Identity())
, x354_(CAABox::Identity())
, x36c_(CAABox::Identity())
, x384_(CVector3f::Zero())
, x400_pointCache(CVector3f::Zero())
, x464_24_growingBeam(growingBeam)
, x464_25_enableTouchDamage(false) {}

rstl::optional_object< CAABox > CBeamProjectile::GetTouchBounds() const {
  if (!GetActive() || !x464_25_enableTouchDamage) {
    return rstl::optional_object_null();
  }
  const CVector3f pos = GetTranslation();
  return CAABox(pos.GetX() - kProjectileBoxAllowance, pos.GetY() - kProjectileBoxAllowance,
                pos.GetZ() - kProjectileBoxAllowance, pos.GetX() + kProjectileBoxAllowance,
                pos.GetY() + kProjectileBoxAllowance, pos.GetZ() + kProjectileBoxAllowance);
}

void CBeamProjectile::CalculateRenderBounds() {
  SetRenderBounds(x354_.GetTransformedAABox(x324_xf));
}

ENTITY_ACCEPT_IMPL(CBeamProjectile)

void CBeamProjectile::Touch(CActor&, CStateManager&) {}

void CBeamProjectile::ResetBeam(CStateManager&, bool) {
  if (x464_24_growingBeam)
    x300_intBeamLength = 0.f;
}

void CBeamProjectile::SetCollisionResultData(EDamageType dType, CRayCastResult& res, TUniqueId id) {
  x2f8_damageType = dType;
  x304_beamLength = res.GetTime();
  x318_collisionPoint = res.GetPoint();
  x30c_collisionNormal = res.GetPlane().GetNormal();
  x2fe_collisionActorId = dType == kDT_Actor ? id : kInvalidUniqueId;
  SetTranslation(res.GetPoint());
}

static inline void ApplyBeamWorldDamage(CBeamProjectile& beam, CStateManager& mgr,
                                        const CVector3f& point, const CDamageInfo& damage,
                                        const CMaterialFilter& filter) {
  const TUniqueId owner = beam.GetOwnerId();
  mgr.ApplyDamageToWorld(owner, beam, point, damage, filter);
}

void CBeamProjectile::UpdateFx(const CTransform4f& xf, float dt, CStateManager& mgr) {
  if (!GetActive())
    return;

  SetTransform(xf.GetRotation());
  if (x464_24_growingBeam) {
    x300_intBeamLength += x308_travelSpeed * dt;
    if (x300_intBeamLength > x2ec_maxLength)
      x300_intBeamLength = x2ec_maxLength;
  }
  x304_beamLength = x300_intBeamLength;
  x2f8_damageType = kDT_None;
  const CVector3f origin = xf.GetTranslation();
  const CVector3f beamEnd =
      xf.GetTranslation() + x300_intBeamLength * xf.GetColumn(kDY).AsNormalized();
  x298_previousPos = origin;
  SetTranslation(beamEnd);

  x354_ = CAABox(-x2f4_beamRadius, 0.f, -x2f4_beamRadius, x2f4_beamRadius, x304_beamLength,
                 x2f4_beamRadius);

  x36c_ = CAABox(CVector3f(-x2f4_beamRadius, 0.f, -x2f4_beamRadius),
                 CVector3f(x2f4_beamRadius, x300_intBeamLength, x2f4_beamRadius))
              .GetTransformedAABox(xf);

  TUniqueId collideId = kInvalidUniqueId;
  TEntityList nearList;
  mgr.BuildNearList(nearList, x36c_,
                    CMaterialFilter::MakeExclude(CMaterialList(kMT_ProjectilePassthrough)), this);

  CRayCastResult res =
      RayCollisionCheckWithWorld(collideId, origin, beamEnd, x300_intBeamLength, nearList, mgr);

  if (TCastToConstPtr< CActor >(mgr.ObjectById(collideId))) {
    SetCollisionResultData(kDT_Actor, res, collideId);
    if (x464_25_enableTouchDamage)
      ApplyDamageToActors(mgr, x12c_curDamageInfo.MakeScaledForTime(dt));

  } else if (res.IsValid()) {
    SetCollisionResultData(kDT_World, res, kInvalidUniqueId);
    if (x464_25_enableTouchDamage) {
      ApplyBeamWorldDamage(*this, mgr, res.GetPoint(), GetCurrentDamageInfo().MakeScaledForTime(dt),
                           GetFilter());
    }
  } else {
    x318_collisionPoint = xf * CVector3f(x2f4_beamRadius, x304_beamLength, x2f4_beamRadius);
    SetTranslation(x318_collisionPoint);
  }
  x324_xf = xf;
}
