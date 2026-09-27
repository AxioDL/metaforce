#ifndef _CBOUNCYGRENADE
#define _CBOUNCYGRENADE

#include "Kyoto/Particles/CElementGen.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "rstl/single_ptr.hpp"

struct SGrenadeVelocityInfo {
private:
  float mMass;
  float mSpeed;

public:
  SGrenadeVelocityInfo(float mass, float speed) : mMass(mass), mSpeed(speed) {}
  explicit SGrenadeVelocityInfo(CInputStream& in)
  : mMass(in.ReadFloat()), mSpeed(in.ReadFloat()) {}

  float GetMass() const { return mMass; }
  float GetSpeed() const { return mSpeed; }
};
CHECK_SIZEOF(SGrenadeVelocityInfo, 0x8)

class CBouncyGrenadeData {
  SGrenadeVelocityInfo mVelocityInfo;
  CDamageInfo mDamageInfo;
  CAssetId mElementGenId1;
  CAssetId mElementGenId2;
  CAssetId mElementGenId3;
  CAssetId mElementGenId4;
  uint mNumBounces;
  ushort mBounceSfx;
  ushort mExplodeSfx;

public:
  CBouncyGrenadeData(const SGrenadeVelocityInfo& velocityInfo, CDamageInfo damageInfo,
                     CAssetId elementGenId1, CAssetId elementGenId2, CAssetId elementGenId3,
                     CAssetId elementGenId4, uint numBounces, ushort bounceSfx, ushort explodeSfx)
  : mVelocityInfo(velocityInfo)
  , mDamageInfo(damageInfo)
  , mElementGenId1(elementGenId1)
  , mElementGenId2(elementGenId2)
  , mElementGenId3(elementGenId3)
  , mElementGenId4(elementGenId4)
  , mNumBounces(numBounces)
  , mBounceSfx(bounceSfx)
  , mExplodeSfx(explodeSfx) {}

  const SGrenadeVelocityInfo& GetVelocityInfo() const { return mVelocityInfo; }
  const CDamageInfo& GetDamageInfo() const { return mDamageInfo; }
  CAssetId GetElementGenId1() const { return mElementGenId1; }
  CAssetId GetElementGenId2() const { return mElementGenId2; }
  CAssetId GetElementGenId3() const { return mElementGenId3; }
  CAssetId GetElementGenId4() const { return mElementGenId4; }
  uint GetNumBounces() const { return mNumBounces; }
  ushort GetBounceSfx() const { return mBounceSfx; }
  ushort GetExplodeSfx() const { return mExplodeSfx; }
};
CHECK_SIZEOF(CBouncyGrenadeData, 0x3c)

class CBouncyGrenade : public CPhysicsActor {
public:
  // CEntity
  ~CBouncyGrenade() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;

  // CActor
  void AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& act, CStateManager& mgr) override;

  // CPhysicsActor
  void CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                    CStateManager& mgr) override;

  CBouncyGrenade(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                 const CTransform4f& xf, const CModelData& mData, const CActorParameters& actParams,
                 TUniqueId parentId, const CBouncyGrenadeData& data, float velocity,
                 float explodePlayerDistance);

private:
  void Explode(CStateManager& mgr, TUniqueId uid);

  CBouncyGrenadeData mData;
  uint mNumBounces;
  TUniqueId mParentId;
  float mElapsedTime;
  rstl::single_ptr< CElementGen > mElementGenCombat;
  rstl::single_ptr< CElementGen > mElementGenXRay;
  rstl::single_ptr< CElementGen > mElementGenThermal;
  rstl::single_ptr< CElementGen > mElementGenTrail;
  float mExplodePlayerDistance;
  bool mExploded : 1;
  bool x2b4_25_ : 1;
};
CHECK_SIZEOF(CBouncyGrenade, (VERSION >= VERSION_GM8E_02 ? 0x2c8 : 0x2b8))

#endif // _CBOUNCYGRENADE
