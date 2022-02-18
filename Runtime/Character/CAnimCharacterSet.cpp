#include "Runtime/Character/CAnimCharacterSet.hpp"

#include "Runtime/CToken.hpp"

namespace metaforce {

CAnimCharacterSet::CAnimCharacterSet(CInputStream& in)
: x0_version(in.ReadShort()), x4_characterSet(in), x1c_animationSet(in) {}

CFactoryFnReturn FAnimCharacterSet(const SObjectTag&, CInputStream& in, const CVParamTransfer&,
                                   CObjectReference* selfRef) {
  return TToken<CAnimCharacterSet>::GetIObjObjectFor(std::make_unique<CAnimCharacterSet>(in));
}

} // namespace metaforce
