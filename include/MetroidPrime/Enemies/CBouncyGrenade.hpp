#ifndef _CBOUNCYGRENADE
#define _CBOUNCYGRENADE

#include "Kyoto/Particles/CElementGen.hpp"
#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "rstl/single_ptr.hpp"

struct SGrenadeVelocityInfo {
private:
  float x0_mass;
  float x4_speed;

public:
  SGrenadeVelocityInfo(float mass, float speed) : x0_mass(mass), x4_speed(speed) {}
  explicit SGrenadeVelocityInfo(CInputStream& in)
  : x0_mass(in.ReadFloat()), x4_speed(in.ReadFloat()) {}

  float GetMass() const { return x0_mass; }
  float GetSpeed() const { return x4_speed; }
};
CHECK_SIZEOF(SGrenadeVelocityInfo, 0x8)

class CBouncyGrenadeData {
  SGrenadeVelocityInfo x0_velocityInfo;
  CDamageInfo x8_damageInfo;
  CAssetId x24_elementGenId1;
  CAssetId x28_elementGenId2;
  CAssetId x2c_elementGenId3;
  CAssetId x30_elementGenId4;
  uint x34_numBounces;
  ushort x38_bounceSfx;
  ushort x3a_explodeSfx;

public:
  CBouncyGrenadeData(const SGrenadeVelocityInfo& velocityInfo, CDamageInfo damageInfo,
                     CAssetId elementGenId1, CAssetId elementGenId2, CAssetId elementGenId3,
                     CAssetId elementGenId4, uint numBounces, ushort bounceSfx, ushort explodeSfx)
  : x0_velocityInfo(velocityInfo)
  , x8_damageInfo(damageInfo)
  , x24_elementGenId1(elementGenId1)
  , x28_elementGenId2(elementGenId2)
  , x2c_elementGenId3(elementGenId3)
  , x30_elementGenId4(elementGenId4)
  , x34_numBounces(numBounces)
  , x38_bounceSfx(bounceSfx)
  , x3a_explodeSfx(explodeSfx) {}

  const SGrenadeVelocityInfo& GetVelocityInfo() const { return x0_velocityInfo; }
  const CDamageInfo& GetDamageInfo() const { return x8_damageInfo; }
  CAssetId GetElementGenId1() const { return x24_elementGenId1; }
  CAssetId GetElementGenId2() const { return x28_elementGenId2; }
  CAssetId GetElementGenId3() const { return x2c_elementGenId3; }
  CAssetId GetElementGenId4() const { return x30_elementGenId4; }
  uint GetNumBounces() const { return x34_numBounces; }
  ushort GetBounceSfx() const { return x38_bounceSfx; }
  ushort GetExplodeSfx() const { return x3a_explodeSfx; }
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

  CBouncyGrenadeData x258_data;
  uint x294_numBounces;
  TUniqueId x298_parentId;
  float x29c_elapsedTime;
  rstl::single_ptr< CElementGen > x2a0_elementGenCombat;
  rstl::single_ptr< CElementGen > x2a4_elementGenXRay;
  rstl::single_ptr< CElementGen > x2a8_elementGenThermal;
  rstl::single_ptr< CElementGen > x2ac_elementGenTrail;
  float x2b0_explodePlayerDistance;
  bool x2b4_24_exploded : 1;
  bool x2b4_25_ : 1;
};
CHECK_SIZEOF(CBouncyGrenade, (VERSION >= VERSION_GM8P_00 ? 0x2c8 : 0x2b8))

#endif // _CBOUNCYGRENADE
