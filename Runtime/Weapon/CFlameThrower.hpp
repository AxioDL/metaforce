#pragma once

#include "Weapon/CGameProjectile.hpp"

namespace urde {
class CFlameInfo;
class CElementGen;
class CFlameThrower : public CGameProjectile {
  static const zeus::CVector3f kLightOffset;
  zeus::CTransform x2e8_;
  zeus::CAABox x318_ = zeus::skNullBox;
  TToken<CGenDescription> x33c_flameDesc;
  std::unique_ptr<CElementGen> x348_flameGen;

  union {
    struct {
      bool x400_25 : 1;
    };
    u32 _dummy = 0;
  };

public:
  CFlameThrower(const TToken<CWeaponDescription>& wDesc, std::string_view name, EWeaponType wType,
                const CFlameInfo& flameInfo, const zeus::CTransform& xf, EMaterialTypes matType,
                const CDamageInfo& dInfo, TUniqueId uid, TAreaId aId, TUniqueId owner, EProjectileAttrib attribs,
                CAssetId w2, s16 sId, CAssetId w3);

  void Accept(IVisitor& visitor);
  void SetTransform(const zeus::CTransform& xf, float);
  void Reset(CStateManager&, bool);
  void Fire(const zeus::CTransform&, CStateManager&, bool);
  bool GetX400_25() const { return x400_25; }
};
} // namespace urde
