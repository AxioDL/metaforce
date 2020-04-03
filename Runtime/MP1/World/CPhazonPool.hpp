#pragma once

#include "Runtime/World/CScriptTrigger.hpp"

#include "Runtime/Particle/CElementGen.hpp"

namespace urde::MP1 {
class CPhazonPool : public CScriptTrigger {
private:
  u32 x164_;
  std::unique_ptr<CModelData> x168_;
  std::unique_ptr<CModelData> x16c_;
  std::unique_ptr<CElementGen> x170_;
  std::unique_ptr<CElementGen> x174_;
  zeus::CAABox x178_;
  zeus::CVector3f x190_;
  float x19c_;
  float x1a0_;
  float x1a4_;
  float x1a8_;
  float x1ac_;
  float x1b0_;
  float x1b4_;
  float x1b8_;
  float x1bc_;
  float x1c0_;
  float x1c4_;
  float x1c8_;
  float x1cc_;
  float x1d0_;
  float x1d4_;
  u32 x1d8_;
  u32 x1dc_;
  bool x1e0_24_ : 1;

public:
  CPhazonPool(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
              const zeus::CVector3f& scale, bool active, CAssetId w1, CAssetId w2, CAssetId w3, CAssetId w4, u32 p11,
              const CDamageInfo& dInfo, const zeus::CVector3f& orientedForce, ETriggerFlags triggerFlags, bool p15,
              float p16, float p17, float p18, float p19);
};
} // namespace urde::MP1
