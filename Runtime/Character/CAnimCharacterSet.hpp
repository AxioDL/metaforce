#pragma once

#include "Runtime/Factory/CFactoryMgr.hpp"
#include "Runtime/Character/CAnimationSet.hpp"
#include "Runtime/Character/CCharacterSet.hpp"

namespace metaforce {

class CAnimCharacterSet {
  u16 x0_version;
  CCharacterSet x4_characterSet;
  CAnimationSet x1c_animationSet;

public:
  explicit CAnimCharacterSet(CInputStream& in);
  const CCharacterSet& GetCharacterSet() const { return x4_characterSet; }
  const CAnimationSet& GetAnimationSet() const { return x1c_animationSet; }
};

CFactoryFnReturn FAnimCharacterSet(const SObjectTag&, CInputStream&, const CVParamTransfer&, CObjectReference* selfRef);

} // namespace metaforce
