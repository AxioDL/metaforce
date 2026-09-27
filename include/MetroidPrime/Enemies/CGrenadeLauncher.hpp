#ifndef _CGRENADELAUNCHER
#define _CGRENADELAUNCHER

#include "Collision/CCollidableSphere.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CDamageVulnerability.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/CPhysicsActor.hpp"
#include "MetroidPrime/Enemies/CBouncyGrenade.hpp"

class CEPGrenadeLaunchParms {
  float mVelocityMin;
  float mVelocityMax;
  float mAngleMin;
  float mAngleMax;

public:
  CEPGrenadeLaunchParms(float velocityMin, float velocityMax, float angleMin, float angleMax)
  : mVelocityMin(velocityMin)
  , mVelocityMax(velocityMax)
  , mAngleMin(angleMin)
  , mAngleMax(angleMax) {}
  explicit CEPGrenadeLaunchParms(CInputStream& in)
  : mVelocityMin(in.ReadFloat())
  , mVelocityMax(in.ReadFloat())
  , mAngleMin(CRelAngle::FromDegrees(in.ReadFloat()).AsRadians())
  , mAngleMax(CRelAngle::FromDegrees(in.ReadFloat()).AsRadians()) {}
  float GetVelocityMin() const { return mVelocityMin; }
  float GetVelocityMax() const { return mVelocityMax; }
  float GetAngleMin() const { return mAngleMin; }
  float GetAngleMax() const { return mAngleMax; }
};
CHECK_SIZEOF(CEPGrenadeLaunchParms, 0x10)

class CEPGrenadeLauncherData {
  CBouncyGrenadeData mGrenadeData;
  CAssetId mGrenadeModelId;
  CAssetId mShootParticleGenDescId;
  ushort mShootSfxId;
  CEPGrenadeLaunchParms mLaunchParms;

public:
  CEPGrenadeLauncherData(const CBouncyGrenadeData& data, CAssetId grenadeModelId,
                         CAssetId shootParticleGenDescId, ushort shootSfxId,
                         const CEPGrenadeLaunchParms& launchParms)
  : mGrenadeData(data)
  , mGrenadeModelId(grenadeModelId)
  , mShootParticleGenDescId(shootParticleGenDescId)
  , mShootSfxId(shootSfxId)
  , mLaunchParms(launchParms) {}
  const CBouncyGrenadeData& GetGrenadeData() const { return mGrenadeData; }
  CAssetId GetGrenadeModelId() const { return mGrenadeModelId; }
  CAssetId GetShootParticleGenDescId() const { return mShootParticleGenDescId; }
  ushort GetShootSfxId() const { return mShootSfxId; }
  const CEPGrenadeLaunchParms& GetLaunchParms() const { return mLaunchParms; }
};
CHECK_SIZEOF(CEPGrenadeLauncherData, 0x58)

class CGrenadeLauncher : public CPhysicsActor {
public:
  ~CGrenadeLauncher() override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Render(const CStateManager& mgr) const override;
  void PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) override;
  void AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const override;
  void Touch(CActor& act, CStateManager& mgr) override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  const CCollisionPrimitive* GetCollisionPrimitive() const override;
  CHealthInfo* HealthInfo(CStateManager& mgr) override { return &mHealthInfo; }
  const CDamageVulnerability* GetDamageVulnerability() const override {
    return &mVulnerability;
  }

  CGrenadeLauncher(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                   const CTransform4f& xf, const CModelData& mData, const CAABox& bounds,
                   const CHealthInfo& healthInfo, const CDamageVulnerability& vulnerability,
                   const CActorParameters& actParams, TUniqueId parentId,
                   const CEPGrenadeLauncherData& data, float explodePlayerDistance);
  void SetAddColor(const CColor& color) { mDamageAddColor = color; }
  void SetVisible(bool visible) { mVisible = visible; }
  void SetFollowPlayer(bool follow) { mFollowPlayer = follow; }
  static CVector3f PredictTargetPosition(const CStateManager& mgr);
  static void ComputeLaunchSpeedAndAngle(const CVector3f& target, const CVector3f& origin,
                                         const CEPGrenadeLaunchParms& parms, float& angleOut,
                                         float& velocityOut);

private:
  static const float skMaxWeight;

  void UpdateLauncherAnimation();
  void LaunchGrenadeProjectile(CStateManager& mgr);
  void UpdateCollisionPrimitive();
  void UpdateGunTracking(float dt, CStateManager& mgr);
  void UpdateGrenadeLauncherDamageTime(float dt);
  void StartExplosionEffect(CStateManager& mgr);
  void UpdateHitDamageTime(float dt);

  int mStarted;
  CHealthInfo mHealthInfo;
  CDamageVulnerability mVulnerability;
  TUniqueId mParentId;
  CEPGrenadeLauncherData mData;
  CCollidableSphere mCSphere;
  float mShotTimer;
  CColor mColor;
  CActorParameters mGrenadeActorParams;
  rstl::optional_object< TLockedToken< CGenDescription > > mParticleGenDesc;
  int mAnimIds[4];
  float mYaw;
  float mYawVelocity;
  float mPitch;
  float mPitchVelocity;
  float mThermalMag;
  float mDamageTimer;
  CColor mDamageColor;
  CColor mDamageAddColor;
  float mExplodePlayerDistance;
  bool mLaunchGrenade;
  bool mVisible;
  bool mFollowPlayer;
};
CHECK_SIZEOF(CGrenadeLauncher, (VERSION >= VERSION_GM8E_02 ? 0x410 : 0x400))
#endif // _CGRENADELAUNCHER
