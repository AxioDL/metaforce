#pragma once

#include "World/CPatterned.hpp"

namespace urde {
class CDamageInfo;

namespace MP1 {
class CBeetle : public CPatterned {
public:
  enum class EEntranceType : u32 { FacePlayer, UseOrientation };

private:
public:
  DEFINE_PATTERNED(Beetle)
  CBeetle(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&, const CPatternedInfo&,
          CPatterned::EFlavorType, EEntranceType, const CDamageInfo&, const CDamageVulnerability&,
          const zeus::CVector3f&, float, float, float, const CDamageVulnerability&, const CActorParameters&,
          const std::experimental::optional<CStaticRes>);

  void Accept(IVisitor& visitor);
};
} // namespace MP1
} // namespace urde
