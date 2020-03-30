#pragma once

#include "Runtime/World/CPhysicsActor.hpp"
#include "Runtime/World/CDamageInfo.hpp"
#include "Runtime/Particle/CElementGen.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

#include <zeus/CTransform.hpp>

namespace urde::MP1 {
struct SGrenadeUnknownStruct {
  float x0_mass;
  float x4_; // speed?

  SGrenadeUnknownStruct(CInputStream& in) : x0_mass(in.readFloatBig()), x4_(in.readFloatBig()) {}
};

struct SBouncyGrenadeData {
  SGrenadeUnknownStruct x0_;
  CDamageInfo x8_damageInfo;
  CAssetId x24_elementGenId1;
  CAssetId x28_elementGenId2;
  CAssetId x2c_elementGenId3;
  CAssetId x30_elementGenId4;
  u32 x34_numBounces;
  u16 x38_bounceSfx;
  u16 x3a_explodeSfx;

  SBouncyGrenadeData(const SGrenadeUnknownStruct& unkStruct, const CDamageInfo& damageInfo, CAssetId w1, CAssetId w2,
                     CAssetId w3, CAssetId w4, u32 w5, u16 s1, u16 s2)
  : x0_(unkStruct)
  , x8_damageInfo(damageInfo)
  , x24_elementGenId1(w1)
  , x28_elementGenId2(w2)
  , x2c_elementGenId3(w3)
  , x30_elementGenId4(w4)
  , x34_numBounces(w5)
  , x38_bounceSfx(s1)
  , x3a_explodeSfx(s2){};
};

class CBouncyGrenade : public CPhysicsActor {
private:
  SBouncyGrenadeData x258_data;
  u32 x294_numBounces;
  TUniqueId x298_parentId;
  float x29c_ = 0.f;
  std::unique_ptr<CElementGen> x2a0_elementGen1;
  std::unique_ptr<CElementGen> x2a4_elementGen2;
  std::unique_ptr<CElementGen> x2a8_elementGen3;
  std::unique_ptr<CElementGen> x2ac_elementGen4;
  float x2b0_explodePlayerDistance;
  bool x2b4_24_exploded : 1;
  bool x2b4_25_ : 1;

public:
  CBouncyGrenade(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                 CModelData&& mData, const CActorParameters& actParams, TUniqueId parentId,
                 const SBouncyGrenadeData& data, float velocity, float explodePlayerDistance);

  void Accept(IVisitor& visitor) override { visitor.Visit(this); }
  void AddToRenderer(const zeus::CFrustum& frustum, const CStateManager& mgr) const override;
  void CollidedWith(TUniqueId id, const CCollisionInfoList &list, CStateManager &mgr) override;
  [[nodiscard]] std::optional<zeus::CAABox> GetTouchBounds() const override;
  void Render(const CStateManager& mgr) const override;
  void Think(float dt, CStateManager& mgr) override;
  void Touch(CActor& act, CStateManager& mgr) override;

private:
  void Explode(CStateManager& mgr, TUniqueId uid);
};
} // namespace urde::MP1
