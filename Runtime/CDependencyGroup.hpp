#pragma once

#include <vector>
#include "Runtime/Factory/CFactoryMgr.hpp"

namespace metaforce {
class CDependencyGroup {
  std::vector<SObjectTag> x0_objectTags;

public:
  explicit CDependencyGroup(CInputStream& in);
  void ReadFromStream(CInputStream& in);
  const std::vector<SObjectTag>& GetObjectTagVector() const { return x0_objectTags; }
};

CFactoryFnReturn FDependencyGroupFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& param,
                                         CObjectReference* selfRef);
} // namespace metaforce
