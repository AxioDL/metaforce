#ifndef _CBEAMPROJECTILE
#define _CBEAMPROJECTILE

#include "types.h"

#include "MetroidPrime/Weapons/CGameProjectile.hpp"

#include "rstl/reserved_vector.hpp"

class CBeamProjectile : public CGameProjectile {
public:
  enum EDamageType {
    kDT_None,
    kDT_Actor,
    kDT_World,
  };

  CBeamProjectile(const TToken<CWeaponDescription>& wDesc, const rstl::string& name, EWeaponType wType,
                  const CTransform4f& xf, int maxLength, float beamRadius, float travelSpeed,
                  EMaterialTypes matType, const CDamageInfo& dInfo, TUniqueId uid, TAreaId aid, TUniqueId owner,
                  EProjectileAttrib attribs, bool growingBeam);

  // CEntity
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  ~CBeamProjectile() override;

  // CActor
  rstl::optional_object<CAABox> GetTouchBounds() const override;
  void Touch(CActor&, CStateManager&) override;
  void CalculateRenderBounds() override;

  // CBeamProjectile
  virtual void UpdateFx(const CTransform4f&, float, CStateManager&);
  virtual void ResetBeam(CStateManager&, bool);
  virtual void Fire(const CTransform4f&, CStateManager&, bool) = 0;
  
  void CauseDamage(const bool b) { mEnableTouchDamage = b; }
  EDamageType GetDamageType() const { return mDamageType; }
  const CVector3f& GetCurrentPos() const { return mCollisionPoint; }
  const CVector3f& GetSurfaceNormal() const { return mCollisionNormal; }
  const CTransform4f& GetBeamTransform() const { return mXf; }
  float GetCurrentLength() const { return mBeamLength; }
  float GetMaxLength() const { return mMaxLength; }
  float GetInvMaxLength() const { return mInvMaxLength; }
  float GetMaxRadius() const { return mBeamRadius; }
  TUniqueId GetCollisionActorId() const { return mCollisionActorId; }
  const rstl::reserved_vector< CVector3f, 8 >& GetPointCache() const { return mPointCache; }
  rstl::reserved_vector< CVector3f, 8 >& PointCache() { return mPointCache; }

private:
  int mIntMaxLength;
  float mMaxLength;
  float mInvMaxLength;
  float mBeamRadius;
  EDamageType mDamageType;
  TUniqueId x2fc_;
  TUniqueId mCollisionActorId;
  float mIntBeamLength;
  float mBeamLength;
  float mTravelSpeed;
  CVector3f mCollisionNormal;
  CVector3f mCollisionPoint;
  CTransform4f mXf;
  CAABox x354_;
  CAABox x36c_;
  rstl::reserved_vector< CVector3f, 10 > x384_;
  rstl::reserved_vector< CVector3f, 8 > mPointCache;
  bool mGrowingBeam : 1;
  bool mEnableTouchDamage : 1;

  void SetCollisionResultData(EDamageType dType, CRayCastResult& res, TUniqueId id);
};
CHECK_SIZEOF(CBeamProjectile, (VERSION >= VERSION_GM8P_00 ? 0x490
                               : VERSION >= VERSION_GM8E_02                             ? 0x478
                                                                                        : 0x468))

#endif // _CBEAMPROJECTILE
