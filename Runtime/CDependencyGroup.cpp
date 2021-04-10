#include "Runtime/CDependencyGroup.hpp"
#include "Runtime/CToken.hpp"

namespace metaforce {
CDependencyGroup::CDependencyGroup(CInputStream& in) { ReadFromStream(in); }

void CDependencyGroup::ReadFromStream(CInputStream& in) {
  u32 depCount = in.readUint32Big();
  x0_objectTags.reserve(depCount);
  for (u32 i = 0; i < depCount; i++)
    x0_objectTags.emplace_back(in);
}

CFactoryFnReturn FDependencyGroupFactory([[maybe_unused]] const SObjectTag& tag, CInputStream& in,
                                         [[maybe_unused]] const CVParamTransfer& param,
                                         [[maybe_unused]] CObjectReference* selfRef) {
  return TToken<CDependencyGroup>::GetIObjObjectFor(std::make_unique<CDependencyGroup>(in));
}

} // namespace metaforce
