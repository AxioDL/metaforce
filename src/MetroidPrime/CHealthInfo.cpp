#include "MetroidPrime/CHealthInfo.hpp"

#include "Kyoto/Streams/CInputStream.hpp"

CHealthInfo::CHealthInfo(CInputStream& in)
{
  in.ReadLong();
  mHealth = in.ReadFloat();
  mKnockbackResistance = in.ReadFloat();
}
