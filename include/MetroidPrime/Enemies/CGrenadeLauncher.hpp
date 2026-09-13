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
  float x0_velocityMin;
  float x4_velocityMax;
  float x8_angleMin;
  float xc_angleMax;

public:
  CEPGrenadeLaunchParms(float velocityMin, float velocityMax, float angleMin, float angleMax)
  : x0_velocityMin(velocityMin)
  , x4_velocityMax(velocityMax)
  , x8_angleMin(angleMin)
  , xc_angleMax(angleMax) {}
  explicit CEPGrenadeLaunchParms(CInputStream& in)
  : x0_velocityMin(in.ReadFloat())
  , x4_velocityMax(in.ReadFloat())
  , x8_angleMin(CRelAngle::FromDegrees(in.ReadFloat()).AsRadians())
  , xc_angleMax(CRelAngle::FromDegrees(in.ReadFloat()).AsRadians()) {}
  float GetVelocityMin() const { return x0_velocityMin; }
  float GetVelocityMax() const { return x4_velocityMax; }
  float GetAngleMin() const { return x8_angleMin; }
  float GetAngleMax() const { return xc_angleMax; }
};
CHECK_SIZEOF(CEPGrenadeLaunchParms, 0x10)

class CEPGrenadeLauncherData {
  CBouncyGrenadeData x0_grenadeData;
  CAssetId x3c_grenadeModelId;
  CAssetId x40_shootParticleGenDescId;
  ushort x44_shootSfxId;
  CEPGrenadeLaunchParms x48_launchParms;

public:
  CEPGrenadeLauncherData(const CBouncyGrenadeData& data, CAssetId grenadeModelId,
                         CAssetId shootParticleGenDescId, ushort shootSfxId,
                         const CEPGrenadeLaunchParms& launchParms)
  : x0_grenadeData(data)
  , x3c_grenadeModelId(grenadeModelId)
  , x40_shootParticleGenDescId(shootParticleGenDescId)
  , x44_shootSfxId(shootSfxId)
  , x48_launchParms(launchParms) {}
  const CBouncyGrenadeData& GetGrenadeData() const { return x0_grenadeData; }
  CAssetId GetGrenadeModelId() const { return x3c_grenadeModelId; }
  CAssetId GetShootParticleGenDescId() const { return x40_shootParticleGenDescId; }
  ushort GetShootSfxId() const { return x44_shootSfxId; }
  const CEPGrenadeLaunchParms& GetLaunchParms() const { return x48_launchParms; }
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
  CHealthInfo* HealthInfo(CStateManager& mgr) override { return &x25c_healthInfo; }
  const CDamageVulnerability* GetDamageVulnerability() const override {
    return &x264_vulnerability;
  }

  CGrenadeLauncher(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                   const CTransform4f& xf, const CModelData& mData, const CAABox& bounds,
                   const CHealthInfo& healthInfo, const CDamageVulnerability& vulnerability,
                   const CActorParameters& actParams, TUniqueId parentId,
                   const CEPGrenadeLauncherData& data, float explodePlayerDistance);
  void SetAddColor(const CColor& color) { x3f4_damageAddColor = color; }
  void SetVisible(bool visible) { x3fd_visible = visible; }
  void SetFollowPlayer(bool follow) { x3fe_followPlayer = follow; }
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

  int x258_started;
  CHealthInfo x25c_healthInfo;
  CDamageVulnerability x264_vulnerability;
  TUniqueId x2cc_parentId;
  CEPGrenadeLauncherData x2d0_data;
  CCollidableSphere x328_cSphere;
  float x348_shotTimer;
  CColor x34c_color;
  CActorParameters x350_grenadeActorParams;
  rstl::optional_object< TLockedToken< CGenDescription > > x3b8_particleGenDesc;
  int x3c8_animIds[4];
  float x3d8_yaw;
  float x3dc_yawVelocity;
  float x3e0_pitch;
  float x3e4_pitchVelocity;
  float x3e8_thermalMag;
  float x3ec_damageTimer;
  CColor x3f0_damageColor;
  CColor x3f4_damageAddColor;
  float x3f8_explodePlayerDistance;
  bool x3fc_launchGrenade;
  bool x3fd_visible;
  bool x3fe_followPlayer;
};
CHECK_SIZEOF(CGrenadeLauncher, 0x400)
#endif // _CGRENADELAUNCHER
