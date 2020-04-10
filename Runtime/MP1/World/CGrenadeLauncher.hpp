#pragma once

#include "Runtime/Character/CModelData.hpp"
#include "Runtime/Collision/CCollidableSphere.hpp"
#include "Runtime/MP1/World/CBouncyGrenade.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CDamageInfo.hpp"
#include "Runtime/World/CDamageVulnerability.hpp"
#include "Runtime/World/CEntityInfo.hpp"
#include "Runtime/World/CHealthInfo.hpp"
#include "Runtime/World/CPhysicsActor.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

#include <array>
#include <string_view>
#include <zeus/CColor.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CQuaternion.hpp>

namespace urde::MP1 {
struct SGrenadeTrajectoryInfo {
private:
  float x0_velocityMin;
  float x4_velocityMax;
  float x8_angleMin;
  float xc_angleMax;

public:
  explicit SGrenadeTrajectoryInfo(CInputStream& in)
  : x0_velocityMin(in.readFloatBig())
  , x4_velocityMax(in.readFloatBig())
  , x8_angleMin(zeus::degToRad(in.readFloatBig()))
  , xc_angleMax(zeus::degToRad(in.readFloatBig())) {}

  [[nodiscard]] float GetVelocityMin() const { return x0_velocityMin; }
  [[nodiscard]] float GetVelocityMax() const { return x4_velocityMax; }
  [[nodiscard]] float GetAngleMin() const { return x8_angleMin; }
  [[nodiscard]] float GetAngleMax() const { return xc_angleMax; }
};

struct SGrenadeLauncherData {
private:
  SBouncyGrenadeData x0_grenadeData;
  CAssetId x3c_grenadeCmdl;
  CAssetId x40_launcherExplodeGenDesc;
  u16 x44_launcherExplodeSfx;
  SGrenadeTrajectoryInfo x48_trajectoryInfo;

public:
  SGrenadeLauncherData(const SBouncyGrenadeData& data, CAssetId w1, CAssetId w2, u16 sfx,
                       const SGrenadeTrajectoryInfo& trajectoryInfo)
  : x0_grenadeData(data)
  , x3c_grenadeCmdl(w1)
  , x40_launcherExplodeGenDesc(w2)
  , x44_launcherExplodeSfx(sfx)
  , x48_trajectoryInfo(trajectoryInfo){};

  [[nodiscard]] const SBouncyGrenadeData& GetGrenadeData() const { return x0_grenadeData; }
  [[nodiscard]] CAssetId GetGrenadeModelId() const { return x3c_grenadeCmdl; }
  [[nodiscard]] CAssetId GetExplosionGenDescId() const { return x40_launcherExplodeGenDesc; }
  [[nodiscard]] u16 GetExplosionSfx() const { return x44_launcherExplodeSfx; }
  [[nodiscard]] const SGrenadeTrajectoryInfo& GetGrenadeTrajectoryInfo() const { return x48_trajectoryInfo; }
};

class CGrenadeLauncher : public CPhysicsActor {
private:
  int x258_started = 0;
  CHealthInfo x25c_healthInfo;
  CDamageVulnerability x264_vulnerability;
  TUniqueId x2cc_parentId;
  SGrenadeLauncherData x2d0_data;
  CCollidableSphere x328_cSphere;
  float x348_shotTimer = -1.f;
  zeus::CColor x34c_color1{1.f};
  CActorParameters x350_grenadeActorParams;
  std::optional<TLockedToken<CGenDescription>> x3b8_particleGenDesc;
  std::array<s32, 4> x3c8_animIds{};
  float x3d8_ = 0.f;
  float x3dc_ = 0.f;
  float x3e0_ = 0.f;
  float x3e4_ = 0.f;
  float x3e8_thermalMag;
  float x3ec_damageTimer = 0.f;
  zeus::CColor x3f0_color2{0.5f, 0.f, 0.f};
  zeus::CColor x3f4_color3{0.f};
  float x3f8_explodePlayerDistance;
  bool x3fc_launchGrenade = false;
  bool x3fd_visible = true;
  bool x3fe_followPlayer = true;

public:
  CGrenadeLauncher(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                   CModelData&& mData, const zeus::CAABox& bounds, const CHealthInfo& healthInfo,
                   const CDamageVulnerability& vulnerability, const CActorParameters& actParams, TUniqueId parentId,
                   const SGrenadeLauncherData& data, float f1);

  void Accept(IVisitor& visitor) override { visitor.Visit(this); }
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) override;
  [[nodiscard]] const CCollisionPrimitive* GetCollisionPrimitive() const override { return &x328_cSphere; }
  [[nodiscard]] const CDamageVulnerability* GetDamageVulnerability() const override { return &x264_vulnerability; }
  [[nodiscard]] std::optional<zeus::CAABox> GetTouchBounds() const override;
  CHealthInfo* HealthInfo(CStateManager& mgr) override { return &x25c_healthInfo; }
  void PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) override;
  void Render(CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void Touch(CActor& act, CStateManager& mgr) override;

  void SetColor(const zeus::CColor& color) { x3f4_color3 = color; }
  void SetVisible(bool val) { x3fd_visible = val; }
  void SetFollowPlayer(bool val) { x3fe_followPlayer = val; }

  static zeus::CVector3f GrenadeTarget(const CStateManager& mgr);
  static void CalculateGrenadeTrajectory(const zeus::CVector3f& target, const zeus::CVector3f& origin,
                                         const SGrenadeTrajectoryInfo& info, float& angleOut, float& velocityOut);

private:
  void UpdateCollision();
  void UpdateColor(float arg);
  void UpdateDamageTime(float arg);
  void CreateExplosion(CStateManager& mgr);
  void UpdateFollowPlayer(CStateManager& mgr, float dt);
  void sub_80230438();
  void LaunchGrenade(CStateManager& mgr);
};
} // namespace urde::MP1
