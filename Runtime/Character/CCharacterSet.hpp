#pragma once

#include <map>

#include "Runtime/IOStreams.hpp"
#include "Runtime/GCNTypes.hpp"
#include "Runtime/Character/CCharacterInfo.hpp"

namespace urde {

class CCharacterSet {
  u16 x0_version;
  std::map<u32, CCharacterInfo> x4_characters;

public:
  CCharacterSet(CInputStream& in);
  const std::map<u32, CCharacterInfo>& GetCharacterInfoMap() const { return x4_characters; }
};

} // namespace urde
