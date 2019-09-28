#pragma once

#include "Runtime/World/CPatterned.hpp"

namespace urde::MP1 {
class CFlyingPirate : public CPatterned {
public:
  DEFINE_PATTERNED(FlyingPirate)

  CFlyingPirate(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
                const CActorParameters&, const CPatternedInfo&, CInputStream&, u32);
};
} // namespace urde::MP1