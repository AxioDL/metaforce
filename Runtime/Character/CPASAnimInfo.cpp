#include "CPASAnimInfo.hpp"

namespace urde
{

CPASAnimInfo::CPASAnimInfo(u32 id, rstl::reserved_vector<CPASAnimParm::UParmValue, 8>&& parms)
: x0_id(id), x4_parms(std::move(parms)) {}

}
