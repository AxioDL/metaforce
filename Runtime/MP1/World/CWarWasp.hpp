#pragma once

#include "World/CPatterned.hpp"

namespace urde {
class CDamageInfo;
namespace MP1 {
class CWarWasp : public CPatterned {
public:
  DEFINE_PATTERNED(WarWasp)
  CWarWasp(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
           CModelData&& mData, const CPatternedInfo& pInfo, CPatterned::EFlavorType flavor, CPatterned::EColliderType,
           const CDamageInfo& dInfo1, const CActorParameters&, CAssetId weapon, const CDamageInfo& dInfo2,
           CAssetId particle, u32 w3);

  void Accept(IVisitor& visitor);
};
} // namespace MP1
} // namespace urde
