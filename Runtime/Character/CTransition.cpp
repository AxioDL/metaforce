#include "Runtime/Character/CTransition.hpp"

namespace metaforce {

CTransition::CTransition(CInputStream& in)
: x0_id(in.ReadLong())
, x4_animA(in.ReadLong())
, x8_animB(in.ReadLong())
, xc_trans(CMetaTransFactory::CreateMetaTrans(in)) {}

} // namespace metaforce
