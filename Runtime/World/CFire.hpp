#pragma once

#include <optional>
#include <memory>

#include "Runtime/World/CActor.hpp"
#include "Runtime/World/CDamageInfo.hpp"

namespace urde {
class CElementGen;
class CFire : public CActor {
  std::unique_ptr<CElementGen> xe8_;
  TUniqueId xec_ownerId;
  CDamageInfo xf0_damageInfo;
  CDamageInfo x10c_damageInfo;
  std::optional<zeus::CAABox> x128_;
  float x144_;
  bool x148_24_ : 1;
  bool x148_25_ : 1;
  bool x148_26_ : 1;
  bool x148_27_ : 1;
  bool x148_28_ : 1 = false;
  bool x148_29_ : 1;
  float x14c_;
  CAssetId x150_;
  float x154_;
  float x158_;
  float x15c_ = 0.f;

public:
  CFire(TToken<CGenDescription>, TUniqueId, TAreaId, bool, TUniqueId, const zeus::CTransform&, const CDamageInfo&,
        const zeus::CAABox&, const zeus::CVector3f&, bool, CAssetId, bool, bool, bool, float, float, float, float);

  void Accept(IVisitor&) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void Think(float, CStateManager&) override;
  std::optional<zeus::CAABox> GetTouchBounds() const override {
    if (GetActive()) {
      return x128_;
    }

    return std::nullopt;
  }

  void Touch(CActor&, CStateManager&) override;
  void AddToRenderer(const zeus::CFrustum&, CStateManager&) override;
};
} // namespace urde