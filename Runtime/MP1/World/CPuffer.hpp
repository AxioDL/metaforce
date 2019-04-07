#pragma once

#include "World/CPatterned.hpp"

namespace urde::MP1 {
class CPuffer : public CPatterned {
  zeus::CVector3f x568_face;
  TToken<CGenDescription> x574_cloudEffect;
  CDamageInfo x57c_cloudDamage;
  bool x598_24_ : 1;
  bool x598_25_ : 1;
  bool x598_26_ : 1;
  s16 x59a_;
  CDamageInfo x59c_explosionDamage;
  float x5b8_;
  CAssetId x5bc_cloudSteam;
  zeus::CVector3f x5c0_move;
  TUniqueId x5cc_ = kInvalidUniqueId;
  s32 x5d0_ = 0;
  rstl::reserved_vector<zeus::CVector3f, 14> x5d4_gasLocators;

  void sub8025bfa4(CStateManager&);

public:
  DEFINE_PATTERNED(Puffer)

  CPuffer(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
          const CActorParameters&, const CPatternedInfo&, float, CAssetId, const CDamageInfo&, CAssetId, float, bool,
          bool, bool, const CDamageInfo&, s16);

  void Accept(IVisitor&);
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
  void Think(float, CStateManager&);
  rstl::optional<zeus::CAABox> GetTouchBounds() const;
  void Touch(CActor&, CStateManager&);
  void Death(CStateManager&, const zeus::CVector3f&, EScriptObjectState);
};
} // namespace urde::MP1
