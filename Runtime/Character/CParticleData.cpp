#include "CParticleData.hpp"

namespace urde {

CParticleData::CParticleData(CInputStream& in)
: x0_duration(in.readUint32Big())
, x4_particle(in)
, xc_boneName(in.readString())
, x1c_scale(in.readFloatBig())
, x20_parentMode(EParentedMode(in.readUint32Big())) {}

} // namespace urde
