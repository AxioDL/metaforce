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
, mIntMaxLength(CMath::AbsI(maxLength))
, mMaxLength(mIntMaxLength)
, mInvMaxLength(1.f / mMaxLength)
, mBeamRadius(beamRadius)
, mDamageType(kDT_None)
, x2fc_(kInvalidUniqueId)
, mCollisionActorId(kInvalidUniqueId)
, mIntBeamLength(growingBeam ? 0.f : mMaxLength)
, mBeamLength(mMaxLength)
, mTravelSpeed(travelSpeed)
, mCollisionNormal(CVector3f::Up())
, mCollisionPoint(CVector3f::Zero())
, mXf(CTransform4f::Identity())
, x354_(CAABox::Identity())
, x36c_(CAABox::Identity())
, x384_(CVector3f::Zero())
, mPointCache(CVector3f::Zero())
, mGrowingBeam(growingBeam)
, mEnableTouchDamage(false) {}

rstl::optional_object< CAABox > CBeamProjectile::GetTouchBounds() const {
  if (!GetActive() || !mEnableTouchDamage) {
    return rstl::optional_object_null();
  }
  const CVector3f pos = GetTranslation();
  return CAABox(pos.GetX() - kProjectileBoxAllowance, pos.GetY() - kProjectileBoxAllowance,
                pos.GetZ() - kProjectileBoxAllowance, pos.GetX() + kProjectileBoxAllowance,
                pos.GetY() + kProjectileBoxAllowance, pos.GetZ() + kProjectileBoxAllowance);
}

void CBeamProjectile::CalculateRenderBounds() {
  SetRenderBounds(x354_.GetTransformedAABox(mXf));
}

ENTITY_ACCEPT_IMPL(CBeamProjectile)

void CBeamProjectile::Touch(CActor&, CStateManager&) {}

void CBeamProjectile::ResetBeam(CStateManager&, bool) {
  if (mGrowingBeam)
    mIntBeamLength = 0.f;
}

void CBeamProjectile::SetCollisionResultData(EDamageType dType, CRayCastResult& res, TUniqueId id) {
  mDamageType = dType;
  mBeamLength = res.GetTime();
  mCollisionPoint = res.GetPoint();
  mCollisionNormal = res.GetPlane().GetNormal();
  mCollisionActorId = dType == kDT_Actor ? id : kInvalidUniqueId;
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
  if (mGrowingBeam) {
    mIntBeamLength += mTravelSpeed * dt;
    if (mIntBeamLength > mMaxLength)
      mIntBeamLength = mMaxLength;
  }
  mBeamLength = mIntBeamLength;
  mDamageType = kDT_None;
  const CVector3f origin = xf.GetTranslation();
  const CVector3f beamEnd =
      xf.GetTranslation() + mIntBeamLength * xf.GetColumn(kDY).AsNormalized();
  mPreviousPos = origin;
  SetTranslation(beamEnd);

  x354_ = CAABox(-mBeamRadius, 0.f, -mBeamRadius, mBeamRadius, mBeamLength,
                 mBeamRadius);

  x36c_ = CAABox(CVector3f(-mBeamRadius, 0.f, -mBeamRadius),
                 CVector3f(mBeamRadius, mIntBeamLength, mBeamRadius))
              .GetTransformedAABox(xf);

  TUniqueId collideId = kInvalidUniqueId;
  TEntityList nearList;
  mgr.BuildNearList(nearList, x36c_,
                    CMaterialFilter::MakeExclude(CMaterialList(kMT_ProjectilePassthrough)), this);

  CRayCastResult res =
      RayCollisionCheckWithWorld(collideId, origin, beamEnd, mIntBeamLength, nearList, mgr);

  if (TCastToConstPtr< CActor >(mgr.ObjectById(collideId))) {
    SetCollisionResultData(kDT_Actor, res, collideId);
    if (mEnableTouchDamage)
      ApplyDamageToActors(mgr, mCurDamageInfo.MakeScaledForTime(dt));

  } else if (res.IsValid()) {
    SetCollisionResultData(kDT_World, res, kInvalidUniqueId);
    if (mEnableTouchDamage) {
      ApplyBeamWorldDamage(*this, mgr, res.GetPoint(), GetCurrentDamageInfo().MakeScaledForTime(dt),
                           GetFilter());
    }
  } else {
    mCollisionPoint = xf * CVector3f(mBeamRadius, mBeamLength, mBeamRadius);
    SetTranslation(mCollisionPoint);
  }
  mXf = xf;
}
