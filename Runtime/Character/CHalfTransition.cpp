#include "Runtime/Character/CHalfTransition.hpp"

#include "Runtime/Character/CMetaTransFactory.hpp"

namespace urde {

CHalfTransition::CHalfTransition(CInputStream& in) {
  x0_id = in.readUint32Big();
  x4_trans = CMetaTransFactory::CreateMetaTrans(in);
}

} // namespace urde
