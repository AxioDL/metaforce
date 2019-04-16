#pragma once

#include "Weapon/CGameProjectile.hpp"
#include "Particle/CFlameWarp.hpp"

namespace urde {
class CFlameInfo;
class CElementGen;
class CFlameThrower : public CGameProjectile {
  static const zeus::CVector3f kLightOffset;
  zeus::CTransform x2e8_;
  zeus::CAABox x318_ = zeus::skNullBox;
  float x32c_ = 0.f;
  float x330_ = 0.f;
  float x334_ = 0.f;
  float x338_;
  TToken<CGenDescription> x33c_flameDesc;
  std::unique_ptr<CElementGen> x348_flameGen;
  CFlameWarp x34c_;
  u32 x3f0_;
  CAssetId x3f4_;
  s16 x3f8_;
  CAssetId x3fc_;

  union {
    struct {
      bool x400_24_ : 1;
      bool x400_25_ : 1;
      bool x400_26_ : 1;
      bool x400_27_ : 1;
    };
    u32 _dummy = 0;
  };

  void CreateFlameParticles(CStateManager&);
public:
  CFlameThrower(const TToken<CWeaponDescription>& wDesc, std::string_view name, EWeaponType wType,
                const CFlameInfo& flameInfo, const zeus::CTransform& xf, EMaterialTypes matType,
                const CDamageInfo& dInfo, TUniqueId uid, TAreaId aId, TUniqueId owner, EProjectileAttrib attribs,
                CAssetId w2, s16 sId, CAssetId w3);

  void Accept(IVisitor& visitor);
  void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const;
  void SetTransform(const zeus::CTransform& xf, float);
  void Reset(CStateManager&, bool);
  void Fire(const zeus::CTransform&, CStateManager&, bool);
  bool GetX400_25() const { return x400_25_; }
};
} // namespace urde
