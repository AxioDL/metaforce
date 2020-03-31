#pragma once

#include "Runtime/World/CPatterned.hpp"
#include "Runtime/World/CVisorFlare.hpp"

namespace urde::MP1 {
class CDrone : public CPatterned {
public:
  DEFINE_PATTERNED(Drone);
  CDrone(TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo& info, const zeus::CTransform& xf,
         float f1, CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms,
         EMovementType movement, EColliderType colliderType, EBodyType bodyType, const CDamageInfo& dInfo1, CAssetId w1,
         const CDamageInfo& dInfo2, CAssetId w2, const std::vector<CVisorFlare::CFlareDef>& flares, float f2, float f3,
         float f4, float f5, float f6, float f7, float f8, float f9, float f10, float f11, float f12, float f13,
         float f14, float f15, float f16, float f17, float f18, float f19, float f20, float f21, float f22, float f23,
         float f24, CAssetId w3, bool b1);

  void Accept(IVisitor& visitor);
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr);
};
} // namespace urde::MP1
