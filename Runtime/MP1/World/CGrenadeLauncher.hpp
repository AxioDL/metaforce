#pragma once

#include "Runtime/Character/CModelData.hpp"
#include "Runtime/Collision/CCollidableSphere.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CDamageInfo.hpp"
#include "Runtime/World/CDamageVulnerability.hpp"
#include "Runtime/World/CEntityInfo.hpp"
#include "Runtime/World/CHealthInfo.hpp"
#include "Runtime/World/CPhysicsActor.hpp"
#include "Runtime/World/CPhysicsActor.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

#include <array>
#include <string_view>
#include <zeus/CColor.hpp>
#include <zeus/CTransform.hpp>
#include <zeus/CQuaternion.hpp>

namespace urde {
namespace MP1 {
struct SGrenadeLauncherData {
  CHealthInfo x0_healthInfo;
  CDamageInfo x8_damageInfo;
  CAssetId x24_;
  CAssetId x28_;
  CAssetId x2c_;
  CAssetId x30_;
  CAssetId x34_;
  u16 x38_;
  u16 x3a_;

  SGrenadeLauncherData(const CHealthInfo& healthInfo, const CDamageInfo& damageInfo, CAssetId w1, CAssetId w2,
                       CAssetId w3, CAssetId w4, CAssetId w5, u16 s1, u16 s2)
  : x0_healthInfo(healthInfo)
  , x8_damageInfo(damageInfo)
  , x24_(w1)
  , x28_(w2)
  , x2c_(w3)
  , x30_(w4)
  , x34_(w5)
  , x38_(s1)
  , x3a_(s2){};
};

class CGrenadeLauncherData {
public:
  SGrenadeLauncherData x0_;
  CAssetId x3c_;
  CAssetId x40_;
  u16 x44_sfx;
  zeus::CQuaternion x48_quat;

  CGrenadeLauncherData(const SGrenadeLauncherData& data, CAssetId w1, CAssetId w2, u16 sfx,
                       const zeus::CQuaternion& quat)
  : x0_(data), x3c_(w1), x40_(w2), x44_sfx(sfx), x48_quat(quat){};
};

class CGrenadeLauncher : public CPhysicsActor {
public:
  int x258_ = 0;
  CHealthInfo x25c_;
  CDamageVulnerability x264_vulnerability;
  TUniqueId x2cc_otherId;
  CGrenadeLauncherData x2d0_data;
  CCollidableSphere x328_cSphere;
  float x348_ = -1.f;
  zeus::CColor x34c_color1{1.f};
  CActorParameters x350_actParms;
  // was TToken<CGenDescription>
  TLockedToken<CGenDescription> x3c0_particleGenDesc;
  std::array<s32, 4> x3c8_animIds;
  float x3d8_ = 0.f;
  float x3dc_ = 0.f;
  float x3e0_ = 0.f;
  float x3e4_ = 0.f;
  float x3e8_thermalMag;
  float x3ec_ = 0.f;
  zeus::CColor x3f0_color2{0.5f, 0.f, 0.f};
  zeus::CColor x3f4_color3{0.f};
  float x3f8_;
  bool x3fc_ = false;
  bool x3fd_ = true;
  bool x3fe_ = true;

  CGrenadeLauncher(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                   CModelData&& mData, const zeus::CAABox& bounds, const CHealthInfo& healthInfo,
                   const CDamageVulnerability& vulnerability, const CActorParameters& actParams, TUniqueId otherId,
                   const CGrenadeLauncherData& data, float f1);

  void Accept(IVisitor& visitor) override { visitor.Visit(this); }
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void AddToRenderer(const zeus::CFrustum& frustum, const CStateManager& mgr) const override;
  const CCollisionPrimitive* GetCollisionPrimitive() const override { return &x328_cSphere; }
  const CDamageVulnerability* GetDamageVulnerability() const override { return &x264_vulnerability; }
  std::optional<zeus::CAABox> GetTouchBounds() const override;
  CHealthInfo* HealthInfo(CStateManager& mgr) override;
  void PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) override;
  void Render(const CStateManager& mgr) const override;
  void Think(float dt, CStateManager& mgr) override;
  void Touch(CActor& act, CStateManager& mgr) override;

protected:
  void UpdateCollision();
  void UpdateColor(float arg);
  void sub_8022f69c(float arg);
  void sub_8022f770(CStateManager& mgr);
  void sub_8022f9e0(CStateManager& mgr, float arg);
  void sub_80230438();
};
} // namespace MP1
} // namespace urde
