#pragma once

#include "Runtime/World/CActor.hpp"
#include "Runtime/World/CDamageInfo.hpp"

namespace metaforce {
class CElementGen;
namespace MP1 {
class CIceAttackProjectile : public CActor {
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
