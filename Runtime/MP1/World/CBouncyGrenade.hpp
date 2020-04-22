#pragma once

#include "Runtime/World/CPhysicsActor.hpp"
#include "Runtime/World/CDamageInfo.hpp"
#include "Runtime/Particle/CElementGen.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

#include <zeus/CTransform.hpp>

namespace urde::MP1 {
struct SGrenadeUnknownStruct {
private:
  float x0_mass;
  float x4_speed; // wrong name probably

public:
  explicit SGrenadeUnknownStruct(CInputStream& in) : x0_mass(in.readFloatBig()), x4_speed(in.readFloatBig()) {}

  [[nodiscard]] float GetMass() const { return x0_mass; }
  [[nodiscard]] float GetSpeed() const { return x4_speed; }
};

struct SBouncyGrenadeData {
private:
  SGrenadeUnknownStruct x0_;
  CDamageInfo x8_damageInfo;
  CAssetId x24_elementGenId1;
  CAssetId x28_elementGenId2;
  CAssetId x2c_elementGenId3;
  CAssetId x30_elementGenId4;
  u32 x34_numBounces;
  u16 x38_bounceSfx;
  u16 x3a_explodeSfx;

public:
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

  [[nodiscard]] const SGrenadeUnknownStruct& GetUnkStruct() const { return x0_; }
  [[nodiscard]] const CDamageInfo& GetDamageInfo() const { return x8_damageInfo; }
  [[nodiscard]] CAssetId GetElementGenId1() const { return x24_elementGenId1; }
  [[nodiscard]] CAssetId GetElementGenId2() const { return x28_elementGenId2; }
  [[nodiscard]] CAssetId GetElementGenId3() const { return x2c_elementGenId3; }
  [[nodiscard]] CAssetId GetElementGenId4() const { return x30_elementGenId4; }
  [[nodiscard]] u32 GetNumBounces() const { return x34_numBounces; }
  [[nodiscard]] u16 GetBounceSfx() const { return x38_bounceSfx; }
  [[nodiscard]] u16 GetExplodeSfx() const { return x3a_explodeSfx; }
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
  bool x2b4_24_exploded : 1 = false;
  bool x2b4_25_ : 1 = false;

public:
  CBouncyGrenade(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                 CModelData&& mData, const CActorParameters& actParams, TUniqueId parentId,
                 const SBouncyGrenadeData& data, float velocity, float explodePlayerDistance);

  void Accept(IVisitor& visitor) override { visitor.Visit(this); }
  void AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) override;
  void CollidedWith(TUniqueId id, const CCollisionInfoList& list, CStateManager& mgr) override;
  [[nodiscard]] std::optional<zeus::CAABox> GetTouchBounds() const override;
  void Render(CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void Touch(CActor& act, CStateManager& mgr) override;

private:
  void Explode(CStateManager& mgr, TUniqueId uid);
};
} // namespace urde::MP1
