#include "CAnimCharacterSet.hpp"
#include "CToken.hpp"

namespace urde {

CAnimCharacterSet::CAnimCharacterSet(CInputStream& in)
: x0_version(in.readUint16Big()), x4_characterSet(in), x1c_animationSet(in) {}

CFactoryFnReturn FAnimCharacterSet(const SObjectTag&, CInputStream& in, const CVParamTransfer&,
                                   CObjectReference* selfRef) {
  return TToken<CAnimCharacterSet>::GetIObjObjectFor(std::make_unique<CAnimCharacterSet>(in));
}

} // namespace urde
