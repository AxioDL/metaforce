#include "Runtime/Character/CParticleData.hpp"

namespace metaforce {

CParticleData::CParticleData(CInputStream& in)
: x0_duration(in.ReadLong())
, x4_particle(in)
, xc_boneName(in.Get<std::string>())
, x1c_scale(in.ReadFloat())
, x20_parentMode(EParentedMode(in.ReadLong())) {}

} // namespace metaforce
