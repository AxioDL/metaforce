#pragma once

#include "World/CPatterned.hpp"
#include "World/CGrappleParameters.hpp"

namespace urde::MP1 {
class CRipper : public CPatterned {
public:
  DEFINE_PATTERNED(Ripper)
  CRipper(TUniqueId uid, std::string_view name, EFlavorType type, const CEntityInfo& info, const zeus::CTransform& xf,
          CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms,
          const CGrappleParameters& grappleParms);
};
}