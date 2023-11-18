#include "Runtime/Character/CHalfTransition.hpp"

#include "Runtime/Character/CMetaTransFactory.hpp"

namespace metaforce {

CHalfTransition::CHalfTransition(CInputStream& in) {
  x0_id = in.ReadLong();
  x4_trans = CMetaTransFactory::CreateMetaTrans(in);
}

} // namespace metaforce
