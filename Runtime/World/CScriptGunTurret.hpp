#pragma once

#include <Runtime/Weapon/CBurstFire.hpp>

#include "CPhysicsActor.hpp"
#include "CDamageInfo.hpp"
#include "CDamageVulnerability.hpp"
#include "Weapon/CProjectileInfo.hpp"
#include "Weapon/CBurstFire.hpp"

namespace urde {
class CCollisionActorManager;
class CScriptGunTurretData {
  friend class CScriptGunTurret;
  float x0_;
  float x4_;
  float x8_;
  float xc_;
  float x10_;
  float x14_;
  float x18_ = 30.f;
  float x1c_;
  float x20_;
  float x24_;
  float x28_;
  float x2c_;
  float x30_;
  float x34_;
  float x38_;
  bool x3c_;
  CAssetId x40_projectileRes;
  CDamageInfo x44_projectileDamage;
  CAssetId x60_;
  CAssetId x64_;
  CAssetId x68_;
  CAssetId x6c_;
  CAssetId x70_;
  CAssetId x74_;
  CAssetId x78_;
  u16 x7c_;
  u16 x7e_;
  u16 x80_unfreezeSound;
  u16 x82_;
  u16 x84_;
  u16 x86_;
  CAssetId x88_;
  float x8c_;
  u32 x90_;
  u32 x94_;
  u32 x98_;
  float x9c_;
  bool xa0_;
  static constexpr s32 skMinProperties = 43;

public:
  CScriptGunTurretData(CInputStream&, s32);
  CAssetId GetPanningEffectRes() const;
  CAssetId GetChargingEffectRes() const;
  CAssetId GetFrozenEffectRes() const;
  CAssetId GetTargettingLightRes() const;
  CAssetId GetDeactivateLightRes() const;
  CAssetId GetIdleLightRes() const;
  const CDamageInfo& GetProjectileDamage() const { return x44_projectileDamage; }
  CAssetId GetProjectileRes() const { return x40_projectileRes; }
  u16 GetUnFreezeSoundId() const;
  float GetIntoDeactivateDelay() const;
  static s32 GetMinProperties() { return skMinProperties; }
};

class CScriptGunTurret : public CPhysicsActor {
  static const SBurst skOOVBurst4InfoTemplate[];
  static const SBurst skOOVBurst3InfoTemplate[];
  static const SBurst skOOVBurst2InfoTemplate[];
  static const SBurst skBurst4InfoTemplate[];
  static const SBurst skBurst3InfoTemplate[];
  static const SBurst skBurst2InfoTemplate[];
  static const SBurst* skBursts[];

public:
  enum class ETurretComponent { Base, Gun };
  enum class ETurretState {

  };

private:
  ETurretComponent x258_type;
  TUniqueId x25c_gunId = kInvalidUniqueId;
  float x260_ = 0.f;
  CHealthInfo x264_healthInfo;
  CDamageVulnerability x26c_damageVuln;
  CScriptGunTurretData x2d4_data;
  TUniqueId x378_ = kInvalidUniqueId;
  CProjectileInfo x37c_projectileInfo;
  CBurstFire x3a4_burstFire;
  zeus::CVector3f x404_;
  TToken<CGenDescription> x410_;
  TToken<CGenDescription> x41c_;
  TToken<CGenDescription> x428_;
  TToken<CGenDescription> x434_;
  TToken<CGenDescription> x440_;
  TToken<CGenDescription> x44c_;
  TLockedToken<CGenDescription> x458_;
  std::unique_ptr<CElementGen> x468_;
  std::unique_ptr<CElementGen> x470_;
  std::unique_ptr<CElementGen> x478_;
  std::unique_ptr<CElementGen> x480_;
  std::unique_ptr<CElementGen> x488_;
  std::unique_ptr<CElementGen> x490_;
  TUniqueId x498_lightId = kInvalidUniqueId;
  std::unique_ptr<CCollisionActorManager> x49c_collisionManager;
  TUniqueId x4a0_ = kInvalidUniqueId;
  rstl::optional<CModelData> x4a4_;
  float x4f4_ = 0.f;
  float x4f8_ = 0.f;
  zeus::CVector3f x4fc_;
  u8 x508_gunSDKSeg = 0xFF;
  CSfxHandle x50c_;
  float x510_ = 0.f;
  zeus::CVector3f x514_;
  s32 x520_ = -1;
  float x524_ = 0.f;
  float x528_ = 0.f;
  float x52c_ = 0.f;
  float x530_ = 0.f;
  float x534_ = 0.f;
  float x538_ = 0.f;
  float x53c_ = 0.f;
  s32 x540_ = -1;
  zeus::CVector3f x544_;
  zeus::CVector3f x550_;
  s32 x55c_ = -1;
  union {
    struct {
      bool x560_24_ : 1;
      bool x560_25_ : 1;
      bool x560_26_ : 1;
      bool x560_27_ : 1;
      bool x560_28_ : 1;
      bool x560_29_ : 1;
      bool x560_30_ : 1;
      bool x560_31_ : 1;
    };
    u32 _dummy = 0;
  };

private:
  void SetupCollisionManager(CStateManager&);

public:
  CScriptGunTurret(TUniqueId uid, std::string_view name, ETurretComponent comp, const CEntityInfo& info,
                   const zeus::CTransform& xf, CModelData&& mData, const zeus::CAABox& aabb, const CHealthInfo& hInfo,
                   const CDamageVulnerability& dVuln, const CActorParameters& aParms,
                   const CScriptGunTurretData& turretData);

  void Accept(IVisitor&);
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
  void Think(float, CStateManager&);
  void Touch(CActor&, CStateManager&);
  void AddToRenderer(const zeus::CFrustum&, const CStateManager&);
  void Render(const CStateManager&) const;
  rstl::optional<zeus::CAABox> GetTouchBounds() const;
  zeus::CVector3f GetOrbitPosition(const CStateManager&) const;
  zeus::CVector3f GetAimPosition(const CStateManager&, float) const;

  CHealthInfo* HealthInfo(CStateManager&) { return &x264_healthInfo; }
  const CDamageVulnerability* GetDamageVulnerability() const { return &x26c_damageVuln; }
  void sub80219b18(s32, CStateManager&);
  void sub8021998c(s32, CStateManager&, float);
  void sub80217408(CStateManager&);
  void sub802172b8(CStateManager&);
  void sub80219a00(float, CStateManager&);
  void sub802189c8();
  void UpdateGunCollisionManager(float, CStateManager&);
  void GetUnFreezeSoundId(float, CStateManager&);
  void UpdateGunParticles(float, CStateManager&);
  void sub80219938(s32, CStateManager&);
  void sub802196c4(s32, CStateManager&, float);
  void sub802195bc(s32, CStateManager&, float);
  void sub8021942c(s32, CStateManager&, float);
  void sub80218f50(s32, CStateManager&, float);
  void sub80218e34(s32, CStateManager&);
  void sub80218bb4(s32, CStateManager&, float);
  bool sub80217ad8(CStateManager&);
  bool sub802179a4(CStateManager&);
  void sub80217f5c(float, CStateManager&);
  zeus::CVector3f sub80217e34(float);
  void sub80216288(float);
  void sub80217124(CStateManager&);
  void sub80218830(float, CStateManager&);
  void sub80216594(CStateManager&);
  bool sub80217950(CStateManager&);
  bool sub80217c24(float);
};
} // namespace urde
