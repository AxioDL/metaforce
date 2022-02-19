#include "Runtime/World/CHealthInfo.hpp"

#include "Runtime/Streams/CInputStream.hpp"
namespace metaforce {

CHealthInfo::CHealthInfo(CInputStream& in) {
  in.ReadLong();
  x0_health = in.ReadFloat();
  x4_knockbackResistance = in.ReadFloat();
}

} // namespace metaforce
