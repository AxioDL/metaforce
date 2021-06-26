#pragma once

#include "Runtime/World/CActor.hpp"
#include "Runtime/World/CDamageInfo.hpp"

namespace metaforce {
class CElementGen;
namespace MP1 {
class CIceAttackProjectile : public CActor {
  class CTrailObject {
    std::unique_ptr<CElementGen> x0_gen1;
    std::unique_ptr<CElementGen> x8_explosion;
    TUniqueId x10_collisionObj;
    float x14_ = 0.f;
    CActorLights x18_ = CActorLights(1, zeus::CVector3f(0.f, 0.f, 1.f), 4, 4, false, false, false, 0.1f);
    zeus::CVector3f x2f8_;
    zeus::CVector3f x304_;
    zeus::CVector3f x310_;
    int x31c_ = 0;
    u8 x320_ = 0;

  public:
    CTrailObject(CElementGen* gen, TUniqueId uid, const zeus::CVector3f& vec1, const zeus::CVector3f& vec2,
                 const zeus::CVector3f& vec3)
    : x0_gen1(gen), x10_collisionObj(uid), x2f8_(vec1), x304_(vec2), x310_(vec3) {}
  };
  TToken<CGenDescription> xe8_;
  TToken<CGenDescription> xf0_;
  TToken<CGenDescription> xf8_;
  std::unique_ptr<CElementGen> x100_;
  std::vector<TUniqueId> x108_trailObjects;
  TUniqueId x118_owner;
  CDamageInfo x11c_;
  CDamageInfo x138_;
  std::optional<zeus::CAABox> x154_;
  float x170_;
  float x174_;
  float x178_ = 0.f;
  float x17c_ = 0.f;
  s32 x180_ = 0;
  CAssetId x184_;
  u16 x188_;
  u16 x18a_;
  CAssetId x18c_;
  bool x190_ = false;
  bool x191_ = false;
  bool x192_ = false;
  int x194_ = 0;

public:
  DEFINE_ENTITY
  CIceAttackProjectile(const TToken<CGenDescription>& gen1, const TToken<CGenDescription>& gen2,
                       const TToken<CGenDescription>& gen3, TUniqueId uid, TAreaId areaId, TUniqueId owner, bool active,
                       const zeus::CTransform& xf, const CDamageInfo& dInfo, const zeus::CAABox& bounds, float f1,
                       float f2, CAssetId unkInt1, u16 unkShort1, u16 unkShort2, CAssetId unkInt2);

  void Accept(IVisitor& visitor) override;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId other, CStateManager& mgr) override;
  void AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) override;
  void Render(CStateManager& mgr) override;

  [[nodiscard]] std::optional<zeus::CAABox> GetTouchBounds() const override {
    if (!GetActive()) {
      return std::nullopt;
    }
    return x154_->getTransformedAABox(GetTransform());
  }

  void Touch(CActor& act, CStateManager& mgr) override;
};
} // namespace MP1
} // namespace metaforce
