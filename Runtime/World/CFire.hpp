#pragma once

#include "CActor.hpp"
#include "CDamageInfo.hpp"

namespace urde {
class CElementGen;
class CFire : public CActor {
  std::unique_ptr<CElementGen> xe8_;
  TUniqueId xec_ownerId;
  CDamageInfo xf0_damageInfo;
  CDamageInfo x10c_damageInfo;
  rstl::optional<zeus::CAABox> x128_;
  float x144_;
  bool x148_24_ : 1;
  bool x148_25_ : 1;
  bool x148_26_ : 1;
  bool x148_27_ : 1;
  bool x148_28_ : 1;
  bool x148_29_ : 1;
  float x14c_;
  CAssetId x150_;
  float x154_;
  float x158_;
  float x15c_ = 0.f;

public:
  CFire(TToken<CGenDescription>, TUniqueId, TAreaId, bool, TUniqueId, const zeus::CTransform&, const CDamageInfo&,
        const zeus::CAABox&, const zeus::CVector3f&, bool, CAssetId, bool, bool, bool, float, float, float, float);

  void Accept(IVisitor&);
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
  void Think(float, CStateManager&);
  rstl::optional<zeus::CAABox> GetTouchBounds() const {
    if (GetActive())
      return x128_;

    return {};
  }

  void Touch(CActor&, CStateManager&);
  void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const;
};
} // namespace urde