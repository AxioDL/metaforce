#ifndef _CSCRIPTPLATFORM
#define _CSCRIPTPLATFORM

#include "types.h"

#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/optional_object.hpp"
#include "rstl/vector.hpp"

class CCollidableOBBTreeGroup;
class CCollidableOBBTreeGroupContainer;
class CFluidPlane;

struct SRiders {
  TUniqueId mUid;
  float mDecayTimer;
  CTransform4f mTransform;

  SRiders(TUniqueId uid)
  : mUid(uid), mDecayTimer(1.f / 6.f), mTransform(CTransform4f::Identity()) {}

  SRiders(TUniqueId uid, float timer, const CTransform4f& xf)
  : mUid(uid), mDecayTimer(timer), mTransform(xf) {}

  bool operator==(const SRiders& other) const { return mUid == other.mUid; }
};

class CScriptPlatform : public CPhysicsActor {
public:
  CScriptPlatform(
      TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
      const CModelData& mData, const CActorParameters& actParams, const CAABox& aabb, float speed,
      bool detectCollision, float xrayAlpha, bool active, const CHealthInfo& hInfo,
      const CDamageVulnerability& dVuln,
      const rstl::optional_object< TLockedToken< CCollidableOBBTreeGroupContainer > >& dcln,
      bool rainSplashes, uint maxRainSplashes, uint rainGenRate);

  // CEntity
  ~CScriptPlatform() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void PreThink(float dt, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  // CActor
  void PreRender(CStateManager&, const CFrustumPlanes&) override;
  void Render(const CStateManager&) const override;
  CHealthInfo* HealthInfo(CStateManager&) override;
  const CDamageVulnerability* GetDamageVulnerability() const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  CVector3f GetOrbitPosition(const CStateManager&) const override;
  CVector3f GetAimPosition(const CStateManager&, float) const override;
  CAABox GetSortingBounds(const CStateManager&) const override;

  // CPhysicsActor
  const CCollisionPrimitive* GetCollisionPrimitive() const override;
  CTransform4f GetPrimitiveTransform() const override;

  // CScriptPlatform
  virtual void SplashThink(const CAABox&, const CFluidPlane&, float, CStateManager&) const;
  virtual CQuaternion Move(float, CStateManager&);

  TUniqueId GetWaypoint(CStateManager& mgr);
  TUniqueId GetNext(TUniqueId uid, CStateManager& mgr);

  bool HasComplexCollision() const { return mTreeGroup.get() != nullptr; }

  bool IsRider(TUniqueId id) const;
  bool IsSlave(TUniqueId id) const;
  void BuildSlaveList(CStateManager& mgr);
  void AddSlave(TUniqueId id, CStateManager& mgr);

  rstl::vector< SRiders >& GetStaticSlaves() { return mSlavesStatic; }
  rstl::vector< SRiders >& GetDynamicSlaves() { return mSlavesDynamic; }

  void SetControlledAnimation(bool controlled) { mControlledAnimation = controlled; }
  void SetDamageVulnerability(const CDamageVulnerability& vuln);
  void SetDisableXRayAlpha(bool value) { mDisableXrayAlpha = value; }
  void SetXRayFog(bool value) { mXrayFog = value; }

  static void AddRider(rstl::vector< SRiders >& riders, TUniqueId riderId,
                       const CPhysicsActor* ridee, CStateManager& mgr);
  static TEntityList BuildNearListFromRiders(CStateManager& mgr,
                                             const rstl::vector< SRiders >& riders);
  static void DecayRiders(rstl::vector< SRiders >& riders, float dt, CStateManager& mgr);
  static void MoveRiders(CStateManager& mgr, float dt, bool active, rstl::vector< SRiders >& riders,
                         rstl::vector< SRiders >& collidedRiders, const CTransform4f& oldXf,
                         const CTransform4f& newXf, const CVector3f& dragDelta,
                         CQuaternion rotDelta);

private:
  typedef rstl::reserved_vector< ushort, 1024 > TMovedList;

  void DragSlave(CStateManager& mgr, TMovedList& moved, CActor* actor, const CVector3f& delta);
  void DragSlaves(CStateManager& mgr, TMovedList& moved, const CVector3f& delta);
  static bool IsInMovedList(TUniqueId id, const TMovedList& moved);

  TUniqueId mCurrentWaypoint;
  TUniqueId mTargetWaypoint;
  float mCurrentSpeed;
  float mMoveDelay;
  float mCollisionRecoverDelay;
  float mFadeInTime;
  float mFadeOutTime;
  CVector3f mDragDelta;
  CQuaternion mRotDelta;
  CHealthInfo mInitialHealth;
  CHealthInfo mHealth;
  CDamageVulnerability mDamageVuln;
  rstl::optional_object< TLockedToken< CCollidableOBBTreeGroupContainer > > mTreeGroupContainer;
  rstl::single_ptr< CCollidableOBBTreeGroup > mTreeGroup;
  rstl::vector< SRiders > mRiders;
  rstl::vector< SRiders > mSlavesStatic;
  rstl::vector< SRiders > mSlavesDynamic;
  float mXrayAlpha;
  uint mMaxRainSplashes;
  uint mRainGenRate;
  TUniqueId mBoundsTrigger;
  bool mDead : 1;
  bool mControlledAnimation : 1;
  bool mDetectCollision : 1;
  bool mSquishedRider : 1;
  bool mRainSplashes : 1;
  bool mSetXrayDrawFlags : 1;
  bool mDisableXrayAlpha : 1;
  bool mXrayFog : 1;
};
CHECK_SIZEOF(CScriptPlatform, (VERSION >= VERSION_GM8E_02 ? 0x368 : 0x358))

#endif // _CSCRIPTPLATFORM
